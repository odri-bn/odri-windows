#include "tsf/text_service.hpp"

#include <algorithm>
#include <cstdio>
#include <new>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include <locale>
#include <iostream>

#include "app/configuration.hpp"
#include "tsf/edit_session.hpp"
#include "util/log.hpp"
#include "windows/module.hpp"
#include "windows/unicode.hpp"

namespace
{

    bool IsSystemModifierPressed()
    {
        return (::GetKeyState(VK_CONTROL) & 0x8000) != 0 ||
               (::GetKeyState(VK_LCONTROL) & 0x8000) != 0 ||
               (::GetKeyState(VK_RCONTROL) & 0x8000) != 0 ||
               (::GetKeyState(VK_LWIN) & 0x8000) != 0 ||
               (::GetKeyState(VK_RWIN) & 0x8000) != 0;
    }

    std::wstring Utf8ToWide(const std::string &utf8)
    {
        if (utf8.empty())
        {
            return {};
        }

        const int length = ::MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
            static_cast<int>(utf8.size()), nullptr, 0);

        if (length <= 0)
        {
            return {};
        }

        std::wstring result(static_cast<std::size_t>(length), L'\0');

        if (::MultiByteToWideChar(
                CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
                static_cast<int>(utf8.size()), result.data(), length) <= 0)
        {
            return {};
        }

        return result;
    }

    // -----------------------------------------------------------------------------
    // Keyboard mapping
    // -----------------------------------------------------------------------------
    //
    // NOTE:
    //
    // 0x0409 is the LANGID for English (United States), but ToUnicodeEx expects
    // an HKL. This is kept here to match your current implementation.
    //
    // We can improve this later by loading the US keyboard layout with
    // LoadKeyboardLayoutW(L"00000409", ...).
    //
    const HKL kUsQwertyHkl = reinterpret_cast<HKL>(static_cast<UINT_PTR>(0x00000409));

    bool KeyboardKeyToLatin(WPARAM wParam, LPARAM lParam, char *latin)
    {
        if (!latin)
        {
            return false;
        }

        *latin = '\0';
        BYTE keyboard_state[256] = {};

        if (!::GetKeyboardState(keyboard_state))
        {
            return false;
        }

        const UINT scan_code = (static_cast<UINT>(lParam) >> 16) & 0xFF;
        wchar_t buffer[8] = {};

        const int result = ::ToUnicodeEx(
            static_cast<UINT>(wParam), scan_code, keyboard_state,
            buffer, ARRAYSIZE(buffer), 1 << 2, kUsQwertyHkl);

        // We only accept one ordinary character.
        //
        // result < 0:
        // dead key
        //
        // result == 0:
        // no character
        //
        // result > 1:
        // multiple UTF-16 code units
        if (result != 1)
        {
            return false;
        }

        // The Odri input buffer is currently byte-based.
        // Therefore only accept ASCII here.
        if (buffer[0] > 0x7F)
        {
            return false;
        }

        *latin = static_cast<char>(buffer[0]);
        return true;
    }

    std::string Hex(unsigned long value)
    {
        char buffer[19] = {};
        std::snprintf(buffer, sizeof(buffer), "0x%08lX", value);
        return buffer;
    }

    std::string WstringToString(const std::wstring &wstr)
    {
        return odri_windows::WideToUtf8(wstr);
    }

    std::string QuoteString(const std::string &value)
    {
        return "'" + value + "'";
    }

    std::string DescribeState(const std::string &latin_buffer,
                             const std::wstring &previous_text,
                             const std::wstring &composition_text)
    {
        return "{ latin_buffer=" + QuoteString(latin_buffer) +
               ", existing=" + QuoteString(WstringToString(previous_text)) +
               ", current=" + QuoteString(WstringToString(composition_text)) + " }";
    }

    std::string DescribeReplacement(const std::wstring &old_text,
                                   const std::wstring &new_text)
    {
        return "{ old=" + QuoteString(WstringToString(old_text)) +
               ", new=" + QuoteString(WstringToString(new_text)) + " }";
    }

    // Minimal read-only edit session: reports whether the current selection
    // is a real selection (non-collapsed) rather than just a caret.
    class SelectionQuerySession : public ITfEditSession
    {
    public:
        explicit SelectionQuerySession(ITfContext *context) : ref_count_(1), context_(context) {}

        bool has_selection() const { return has_selection_; }

        // IUnknown
        STDMETHODIMP QueryInterface(REFIID riid, void **ppv) override
        {
            if (!ppv)
            {
                return E_INVALIDARG;
            }
            *ppv = nullptr;
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfEditSession))
            {
                *ppv = static_cast<ITfEditSession *>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHODIMP_(ULONG)
        AddRef() override
        {
            return static_cast<ULONG>(::InterlockedIncrement(&ref_count_));
        }

        STDMETHODIMP_(ULONG)
        Release() override
        {
            const LONG remaining = ::InterlockedDecrement(&ref_count_);
            if (remaining == 0)
            {
                delete this;
            }
            return static_cast<ULONG>(remaining);
        }

        // ITfEditSession
        STDMETHODIMP DoEditSession(TfEditCookie edit_cookie) override
        {
            TF_SELECTION selection{};
            ULONG fetched = 0;
            HRESULT hr = context_->GetSelection(edit_cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched);
            if (FAILED(hr) || fetched == 0 || !selection.range)
            {
                return hr;
            }

            BOOL is_empty = TRUE;
            hr = selection.range->IsEmpty(edit_cookie, &is_empty);
            selection.range->Release();
            if (SUCCEEDED(hr))
            {
                has_selection_ = (is_empty == FALSE);
            }
            return hr;
        }

    private:
        LONG ref_count_;
        ITfContext *context_;
        bool has_selection_ = false;
    };

    // True when the user has highlighted text (not just a caret).
    bool HasNonEmptySelection(ITfContext *context, TfClientId client_id)
    {
        if (!context)
        {
            return false;
        }

        auto *session = new (std::nothrow) SelectionQuerySession(context);
        if (!session)
        {
            return false;
        }

        HRESULT session_result = E_FAIL;
        const HRESULT hr = context->RequestEditSession(
            client_id, session, TF_ES_SYNC | TF_ES_READ, &session_result);

        const bool result = SUCCEEDED(hr) && SUCCEEDED(session_result) && session->has_selection();
        session->Release();
        return result;
    }

} // namespace

namespace odri_windows
{

    // =============================================================================
    // Helpers
    // =============================================================================
    bool IsNavigationKey(WPARAM wParam)
    {
        switch (wParam)
        {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
            return true;
        default:
            return false;
        }
    }

    // =============================================================================
    // Construction / destruction
    // =============================================================================
    OdriTextService::OdriTextService() : ref_count_(1)
    {
        ModuleAddRef();
    }

    OdriTextService::~OdriTextService()
    {
        DetachThreadManager();
        ModuleRelease();
    }

    // =============================================================================
    // IUnknown
    // =============================================================================
    STDMETHODIMP OdriTextService::QueryInterface(REFIID riid, void **ppv)
    {
        if (!ppv)
        {
            return E_INVALIDARG;
        }

        *ppv = nullptr;

        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
        {
            *ppv = static_cast<ITfTextInputProcessor *>(this);
        }
        else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
        {
            *ppv = static_cast<ITfTextInputProcessorEx *>(this);
        }
        else if (IsEqualIID(riid, IID_ITfKeyEventSink))
        {
            *ppv = static_cast<ITfKeyEventSink *>(this);
        }

        if (!*ppv)
        {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG)
    OdriTextService::AddRef()
    {
        return static_cast<ULONG>(::InterlockedIncrement(&ref_count_));
    }

    STDMETHODIMP_(ULONG)
    OdriTextService::Release()
    {
        const LONG remaining = ::InterlockedDecrement(&ref_count_);
        if (remaining == 0)
        {
            delete this;
        }
        return static_cast<ULONG>(remaining);
    }

    // =============================================================================
    // ITfTextInputProcessor
    // =============================================================================
    STDMETHODIMP OdriTextService::Activate(ITfThreadMgr *thread_mgr, TfClientId client_id)
    {
        return ActivateEx(thread_mgr, client_id, 0);
    }

    STDMETHODIMP OdriTextService::ActivateEx(ITfThreadMgr *thread_mgr, TfClientId client_id, DWORD flags)
    {
        log::Initialize();
        ODRI_LOG_INFO("tsf activation requested");

        const HRESULT hr = AttachThreadManager(thread_mgr, client_id, flags);
        if (FAILED(hr))
        {
            ODRI_LOG_ERROR("tsf activation failed hr=" + Hex(static_cast<unsigned long>(hr)));
            DetachThreadManager();
        }

        return hr;
    }

    STDMETHODIMP OdriTextService::Deactivate()
    {
        ODRI_LOG_INFO("tsf deactivated");
        DetachThreadManager();
        return S_OK;
    }

    // =============================================================================
    // TSF attachment
    // =============================================================================
    HRESULT OdriTextService::AttachThreadManager(ITfThreadMgr *thread_mgr, TfClientId client_id, DWORD flags)
    {
        if (!thread_mgr)
        {
            return E_INVALIDARG;
        }

        thread_mgr_ = thread_mgr;
        client_id_ = client_id;
        activate_flags_ = flags;

        ODRI_LOG_INFO("tsf attached client_id=" + std::to_string(client_id) + " flags=" + Hex(flags));

        if (flags & TF_TMAE_SECUREMODE)
        {
            ODRI_LOG_INFO("tsf secure desktop");
        }

        HRESULT hr = thread_mgr_->QueryInterface(IID_PPV_ARGS(&keystroke_mgr_));
        if (FAILED(hr))
        {
            ODRI_LOG_ERROR("failed to obtain ITfKeystrokeMgr hr=" + Hex(static_cast<unsigned long>(hr)));
            return hr;
        }

        hr = keystroke_mgr_->AdviseKeyEventSink(client_id_, static_cast<ITfKeyEventSink *>(this), TRUE);
        if (FAILED(hr))
        {
            ODRI_LOG_ERROR("AdviseKeyEventSink failed hr=" + Hex(static_cast<unsigned long>(hr)));
            keystroke_mgr_.Reset();
            return hr;
        }

        ODRI_LOG_INFO("tsf key sink advised");
        return S_OK;
    }

    void OdriTextService::DetachThreadManager()
    {
        if (keystroke_mgr_)
        {
            keystroke_mgr_->UnadviseKeyEventSink(client_id_);
            keystroke_mgr_.Reset();
            ODRI_LOG_INFO("tsf key sink removed");
        }

        ResetOdriState();
        client_id_ = TF_CLIENTID_NULL;
        activate_flags_ = 0;

        if (thread_mgr_)
        {
            thread_mgr_.Reset();
            ODRI_LOG_INFO("tsf thread manager detached");
        }
    }

    // =============================================================================
    // Odri state
    // =============================================================================
    void OdriTextService::ResetOdriState()
    {
        ODRI_LOG_INFO("tsf state reset");
        latin_buffer_.clear();
        composition_text_.clear();
        previous_text_.clear();
        previous_units_.clear();
        owned_range_.Reset();
        active_context_.Reset();
    }

    // =============================================================================
    // Focus
    // =============================================================================
    STDMETHODIMP OdriTextService::OnSetFocus(BOOL foreground)
    {
        ODRI_LOG_INFO("on_set_focus foreground=" + std::string(foreground ? "true" : "false"));
        if (!foreground)
        {
            ResetOdriState();
        }
        return S_OK;
    }

    // =============================================================================
    // Key testing
    // =============================================================================
    STDMETHODIMP OdriTextService::OnTestKeyDown(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *eaten)
    {
        (void)context;
        (void)lParam;
        if (!eaten)
        {
            return E_INVALIDARG;
        }
        *eaten = FALSE;

        if (IsSystemModifierPressed())
        {
            return S_OK;
        }

        // -------------------------------------------------------------------------
        // Backspace
        // -------------------------------------------------------------------------
        if (wParam == VK_BACK)
        {
            // Only claim Backspace while we have a Latin buffer to shorten
            // AND nothing is selected (a selection must be deleted by the app).
            *eaten = (!latin_buffer_.empty() && !HasNonEmptySelection(context, client_id_)) ? TRUE : FALSE;
            return S_OK;
        }

        // -------------------------------------------------------------------------
        // Space / Enter
        // -------------------------------------------------------------------------
        if (wParam == VK_SPACE || wParam == VK_RETURN)
        {
            *eaten = TRUE;
            return S_OK;
        }

        // -------------------------------------------------------------------------
        // US QWERTY character
        // -------------------------------------------------------------------------
        char latin = 0;
        if (KeyboardKeyToLatin(wParam, lParam, &latin))
        {
            *eaten = TRUE;
            ODRI_LOG_INFO("test key accepted: latin=" + std::string(1, latin) + " state=" + DescribeState(std::string(latin_buffer_.begin(), latin_buffer_.end()), previous_text_, composition_text_));
        }
        else
        {
            ODRI_LOG_INFO("test key ignored: no transliterable character");
        }

        return S_OK;
    }

    STDMETHODIMP OdriTextService::OnTestKeyUp(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *eaten)
    {
        (void)context;
        (void)lParam;
        if (!eaten)
        {
            return E_INVALIDARG;
        }
        *eaten = FALSE;
        ODRI_LOG_INFO("test key up: state unchanged");
        return S_OK;
    }

    // =============================================================================
    // Key down
    // =============================================================================
    STDMETHODIMP OdriTextService::OnKeyDown(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *eaten)
    {
        if (!eaten)
        {
            return E_INVALIDARG;
        }
        *eaten = FALSE;

        if (IsSystemModifierPressed())
        {
            return S_OK;
        }
        if (!context)
        {
            return E_INVALIDARG;
        }

        ODRI_LOG_INFO("key down: evaluating edit state");

        // =========================================================================
        // Backspace
        // =========================================================================
        if (wParam == VK_BACK)
        {
            if (latin_buffer_.empty())
            {
                ODRI_LOG_INFO("backspace: no pending latin, leaving app text alone");
                *eaten = FALSE;
                return S_OK;
            }

            // Text is selected: let the app delete the selection normally and
            // forget our word, since the selection may cover or replace it.
            if (HasNonEmptySelection(context, client_id_))
            {
                ODRI_LOG_INFO("backspace: selection is active, clearing Odri state");
                ResetOdriState();
                *eaten = FALSE;
                return S_OK;
            }

            latin_buffer_.pop_back();
            ODRI_LOG_INFO("backspace: latin buffer reduced to " + QuoteString(std::string(latin_buffer_.begin(), latin_buffer_.end())) +
                            " state=" + DescribeState(std::string(latin_buffer_.begin(), latin_buffer_.end()), previous_text_, composition_text_));

            // We handled it. Never let the app also delete a character.
            *eaten = TRUE;

            const HRESULT hr = RunEditSession(context, CompositionEditOperation::Update, '\0');
            if (FAILED(hr))
            {
                ODRI_LOG_ERROR("backspace edit session failed hr=" + Hex(static_cast<unsigned long>(hr)));
            }
            return hr;
        }

        // =========================================================================
        // Space / Enter
        // =========================================================================
        if (wParam == VK_SPACE || wParam == VK_RETURN)
        {
            ODRI_LOG_INFO("space/enter closes the current Odri word; state reset");
            ResetOdriState();
            *eaten = FALSE;
            return S_OK;
        }

        // =========================================================================
        // Navigation
        // =========================================================================
        if (IsNavigationKey(wParam))
        {
            ODRI_LOG_INFO("navigation key entered; dropping pending Odri state");
            ResetOdriState();
            *eaten = FALSE;
            return S_OK;
        }

        // =========================================================================
        // US QWERTY character
        // =========================================================================
        char latin = 0;
        if (!KeyboardKeyToLatin(wParam, lParam, &latin))
        {
            ODRI_LOG_INFO("key ignored: not a transliterable character");
            return S_OK;
        }

        // -------------------------------------------------------------------------
        // Append Latin character to our authoritative buffer and re-run
        // transliteration inside a single edit session.
        // -------------------------------------------------------------------------
        latin_buffer_.push_back(latin);

        // Odri has claimed this key.
        // Never let Windows insert the original Latin character.
        *eaten = TRUE;

        const HRESULT hr = RunEditSession(context, CompositionEditOperation::Update, latin);
        if (FAILED(hr))
        {
            ODRI_LOG_ERROR("update edit session failed hr=" + Hex(static_cast<unsigned long>(hr)) + " state=" + DescribeState(std::string(latin_buffer_.begin(), latin_buffer_.end()), previous_text_, composition_text_));
        }

        return hr;
    }

    // =============================================================================
    // Key up
    // =============================================================================
    STDMETHODIMP OdriTextService::OnKeyUp(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *eaten)
    {
        (void)context;
        (void)lParam;
        if (!eaten)
        {
            return E_INVALIDARG;
        }
        *eaten = FALSE;
        ODRI_LOG_INFO("key up: state unchanged");
        return S_OK;
    }

    // =============================================================================
    // Preserved key
    // =============================================================================
    STDMETHODIMP OdriTextService::OnPreservedKey(ITfContext *context, REFGUID rguid, BOOL *eaten)
    {
        (void)context;
        if (!eaten)
        {
            return E_INVALIDARG;
        }
        *eaten = FALSE;
        ODRI_LOG_INFO("preserved key: no state change");
        return S_OK;
    }

    // =============================================================================
    // Check whether our owned range is still immediately before the caret
    // =============================================================================
    bool OdriTextService::IsOwnedRangeAtSelection(ITfContext *context, TfEditCookie edit_cookie) const
    {
        if (!context || !owned_range_)
        {
            return false;
        }

        // The range belongs to the context in which it was created.
        if (active_context_.Get() != context)
        {
            return false;
        }

        TF_SELECTION selection{};
        ULONG fetched = 0;

        HRESULT hr = context->GetSelection(edit_cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched);
        if (FAILED(hr) || fetched == 0 || !selection.range)
        {
            return false;
        }

        // The user's selection must be a collapsed caret.
        BOOL selection_empty = FALSE;
        hr = selection.range->IsEmpty(edit_cookie, &selection_empty);
        if (FAILED(hr) || !selection_empty)
        {
            selection.range->Release();
            return false;
        }

        // The caret must be exactly at the end of our owned range.
        BOOL equal_end = FALSE;
        hr = selection.range->IsEqualEnd(edit_cookie, owned_range_.Get(), TF_ANCHOR_END, &equal_end);
        selection.range->Release();

        if (FAILED(hr))
        {
            return false;
        }

        return equal_end != FALSE;
    }

    // =============================================================================
    // Shared edit-session dispatch
    // =============================================================================
    HRESULT OdriTextService::RunEditSession(ITfContext *context, CompositionEditOperation operation, char latin)
    {
        if (!context)
        {
            return E_INVALIDARG;
        }

        auto *session = new (std::nothrow) CompositionEditSession(this, context, operation, latin);
        if (!session)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT session_result = E_FAIL;
        const HRESULT hr = context->RequestEditSession(client_id_, session, TF_ES_SYNC | TF_ES_READWRITE, &session_result);

        session->Release();

        if (FAILED(hr))
        {
            return hr;
        }

        return session_result;
    }

    // =============================================================================
    // Text diff (UTF-16)
    //
    // Diffs the final rendered text rather than the semantic units, so that
    // "ক" -> "কি" is treated as a pure append of "ি" instead of replacing "ক".
    // =============================================================================
    struct TextDiff
    {
        size_t prefix;      // UTF-16 code units unchanged at the start
        size_t suffix;      // UTF-16 code units unchanged at the end
        size_t old_changed; // length of the removed part in the old text
        size_t new_changed; // length of the inserted part in the new text
    };

    static bool IsHighSurrogate(wchar_t c) { return c >= 0xD800 && c <= 0xDBFF; }
    static bool IsLowSurrogate(wchar_t c) { return c >= 0xDC00 && c <= 0xDFFF; }

    static TextDiff DiffText(const std::wstring &old_text, const std::wstring &new_text)
    {
        size_t prefix = 0;
        const size_t min_len = (std::min)(old_text.size(), new_text.size());
        while (prefix < min_len && old_text[prefix] == new_text[prefix])
        {
            ++prefix;
        }
        // Never end the shared prefix in the middle of a surrogate pair.
        if (prefix > 0 && IsHighSurrogate(old_text[prefix - 1]))
        {
            --prefix;
        }

        size_t suffix = 0;
        while (suffix < old_text.size() - prefix &&
               suffix < new_text.size() - prefix &&
               old_text[old_text.size() - 1 - suffix] == new_text[new_text.size() - 1 - suffix])
        {
            ++suffix;
        }
        // Never start the shared suffix in the middle of a surrogate pair.
        if (suffix > 0 && IsLowSurrogate(old_text[old_text.size() - suffix]))
        {
            --suffix;
        }

        // If nothing at the start is shared, drop the shared suffix too.
        //
        // Keeping it would turn the edit into an insertion at the LEFT boundary
        // of owned_range_ (e.g. "ত" -> "্ত"), where range gravity decides whether
        // the new text ends up inside our range. Replacing the whole range is
        // unambiguous and also matches the semantics: the unit itself changed.
        //
        // The same applies to mid-word insertions (e.g. "কম" -> "ক্ম"): inserting
        // into an empty range between two existing characters is unreliable in
        // some apps. So we always keep only the shared PREFIX and rewrite the
        // tail with a normal replace of a non-empty range (or an append at the end).
        suffix = 0;

        return {prefix, suffix,
                old_text.size() - prefix - suffix,
                new_text.size() - prefix - suffix};
    }

    // =============================================================================
    // Perform committed-range replacement
    // =============================================================================
    HRESULT OdriTextService::DoCompositionUpdate(ITfContext *context, TfEditCookie edit_cookie, char latin)
    {
        static_cast<void>(latin);

        if (!context)
        {
            return E_INVALIDARG;
        }

        std::vector<EngineUnit> current_units;
        if (!engine_.ConvertUnits(latin_buffer_, &current_units))
        {
            ODRI_LOG_ERROR("tsf unit conversion failed");
            return E_FAIL;
        }

        if (current_units == previous_units_)
        {
            ODRI_LOG_INFO("tsf update skipped: units unchanged");
            return S_OK;
        }

        // -------------------------------------------------------------------------
        // Build the complete UTF-8 output from the semantic units.
        //
        // The TSF layer does not need to understand Bangla orthography.
        // Odri has already determined the semantic boundaries.
        // -------------------------------------------------------------------------
        std::string bangla_utf8;
        for (const auto &unit : current_units)
        {
            bangla_utf8 += unit.text;
        }

        std::wstring bangla = Utf8ToWide(bangla_utf8);
        if (!bangla_utf8.empty() && bangla.empty())
        {
            ODRI_LOG_ERROR("tsf utf-8 to utf-16 conversion failed");
            return E_FAIL;
        }
        composition_text_ = std::move(bangla);

        ODRI_LOG_INFO("tsf state " + DescribeState(std::string(latin_buffer_.begin(), latin_buffer_.end()), previous_text_, composition_text_));

        // -------------------------------------------------------------------------
        // Empty output means delete our previously committed output.
        // -------------------------------------------------------------------------
        if (composition_text_.empty())
        {
            if (owned_range_)
            {
                HRESULT hr = owned_range_->SetText(edit_cookie, 0, L"", 0);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
            composition_text_.clear();
            previous_text_.clear();
            previous_units_.clear();
            if (latin_buffer_.empty())
            {
                ResetOdriState();
            }
            else
            {
                owned_range_.Reset();
                active_context_.Reset();
            }
            return S_OK;
        }

        // -------------------------------------------------------------------------
        // First character/word:
        //
        // Insert the Bangla output and keep the exact returned range.
        // -------------------------------------------------------------------------
        if (!owned_range_)
        {
            ODRI_LOG_INFO("inserting first output" + (latin_buffer_.empty() ? "" : " latin_buffer=" + QuoteString(std::string(latin_buffer_.begin(), latin_buffer_.end()))));
            Microsoft::WRL::ComPtr<ITfInsertAtSelection> insert_at_selection;

            HRESULT hr = context->QueryInterface(IID_PPV_ARGS(&insert_at_selection));
            if (FAILED(hr))
            {
                ODRI_LOG_ERROR("QueryInterface(ITfInsertAtSelection) failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                return hr;
            }

            Microsoft::WRL::ComPtr<ITfRange> inserted_range;
            hr = insert_at_selection->InsertTextAtSelection(
                edit_cookie, 0, composition_text_.c_str(),
                static_cast<LONG>(composition_text_.size()), inserted_range.GetAddressOf());

            if (FAILED(hr))
            {
                return hr;
            }

            if (!inserted_range)
            {
                ODRI_LOG_ERROR("InsertTextAtSelection returned NULL range");
                return E_UNEXPECTED;
            }

            owned_range_ = std::move(inserted_range);
            active_context_ = context;
        }
        else
        {
            // ---------------------------------------------------------------------
            // Existing Odri range.
            //
            // Diff the final UTF-16 text (not the semantic units) and replace only
            // the region that actually changed. For "ক" -> "কি" this means the
            // existing "ক" is untouched and only "ি" is appended.
            // ---------------------------------------------------------------------
            const TextDiff diff = DiffText(previous_text_, composition_text_);

            const std::wstring old_changed = previous_text_.substr(diff.prefix, diff.old_changed);
            const std::wstring new_changed = composition_text_.substr(diff.prefix, diff.new_changed);

            ODRI_LOG_INFO("tsf diff prefix=" + std::to_string(diff.prefix) +
                            " suffix=" + std::to_string(diff.suffix) +
                            " replacement=" + DescribeReplacement(old_changed, new_changed));

            // Clone the complete Odri range.
            Microsoft::WRL::ComPtr<ITfRange> changed_range;
            HRESULT hr = owned_range_->Clone(changed_range.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }
            if (!changed_range)
            {
                ODRI_LOG_ERROR("owned_range Clone returned NULL range");
                return E_UNEXPECTED;
            }

            // ---------------------------------------------------------------------
            // Move the start forward past the unchanged prefix.
            //
            // SDK 10.0.26100.0 also returns the number of characters moved.
            // ---------------------------------------------------------------------
            LONG moved = 0;
            hr = changed_range->ShiftStart(edit_cookie, static_cast<LONG>(diff.prefix), &moved, nullptr);
            if (FAILED(hr))
            {
                ODRI_LOG_ERROR("ShiftStart failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                return hr;
            }

            // ---------------------------------------------------------------------
            // Move the end backward past the unchanged suffix.
            //
            // A negative count moves the end toward the beginning.
            // ---------------------------------------------------------------------
            moved = 0;
            hr = changed_range->ShiftEnd(edit_cookie, -static_cast<LONG>(diff.suffix), &moved, nullptr);
            if (FAILED(hr))
            {
                ODRI_LOG_ERROR("ShiftEnd failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                return hr;
            }

            // ---------------------------------------------------------------------
            // Build only the newly changed portion.
            // ---------------------------------------------------------------------
            const std::wstring replacement = composition_text_.substr(diff.prefix, diff.new_changed);

            // ---------------------------------------------------------------------
            // Replace only the changed range.
            //
            // The changed range may be empty (pure append) and replacement may be
            // empty (pure deletion).
            // ---------------------------------------------------------------------
            ODRI_LOG_INFO("tsf replacement " + DescribeReplacement(
                                previous_text_.substr(diff.prefix, diff.old_changed),
                                composition_text_.substr(diff.prefix, diff.new_changed)) +
                            " latin_buffer=" + QuoteString(std::string(latin_buffer_.begin(), latin_buffer_.end())));
            hr = changed_range->SetText(edit_cookie, 0, replacement.c_str(), static_cast<LONG>(replacement.size()));
            if (FAILED(hr))
            {
                ODRI_LOG_ERROR("partial SetText failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                return hr;
            }

            // ---------------------------------------------------------------------
            // Re-anchor the end of owned_range_ explicitly.
            //
            // After SetText, changed_range covers the newly written text. The owned
            // range must end at the end of that text plus the unchanged suffix.
            // Doing this explicitly means we do not depend on range gravity when
            // text is appended at the very end (the "ক" + "ি" case).
            // ---------------------------------------------------------------------
            hr = owned_range_->ShiftEndToRange(edit_cookie, changed_range.Get(), TF_ANCHOR_END);
            if (FAILED(hr))
            {
                ODRI_LOG_ERROR("ShiftEndToRange failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                return hr;
            }

            // When the whole range was replaced (prefix == 0), the start of
            // owned_range_ must also follow the newly written text.
            if (diff.prefix == 0)
            {
                hr = owned_range_->ShiftStartToRange(edit_cookie, changed_range.Get(), TF_ANCHOR_START);
                if (FAILED(hr))
                {
                    ODRI_LOG_ERROR("ShiftStartToRange failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                    return hr;
                }
            }

            if (diff.suffix > 0)
            {
                moved = 0;
                hr = owned_range_->ShiftEnd(edit_cookie, static_cast<LONG>(diff.suffix), &moved, nullptr);
                if (FAILED(hr))
                {
                    ODRI_LOG_ERROR("ShiftEnd (suffix restore) failed, hr=" + Hex(static_cast<unsigned long>(hr)));
                    return hr;
                }
            }
        }

        // -------------------------------------------------------------------------
        // Save semantic units and rendered text only after the TSF update succeeded.
        // -------------------------------------------------------------------------
        previous_units_ = std::move(current_units);
        previous_text_ = composition_text_;

        // -------------------------------------------------------------------------
        // Clone the owned range.
        //
        // IMPORTANT:
        // Do NOT collapse owned_range_ itself.
        // -------------------------------------------------------------------------
        Microsoft::WRL::ComPtr<ITfRange> caret_range;
        HRESULT hr = owned_range_->Clone(caret_range.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        if (!caret_range)
        {
            ODRI_LOG_ERROR("owned_range Clone returned NULL range");
            return E_UNEXPECTED;
        }

        // -------------------------------------------------------------------------
        // Collapse ONLY the temporary caret range.
        // -------------------------------------------------------------------------
        hr = caret_range->Collapse(edit_cookie, TF_ANCHOR_END);
        if (FAILED(hr))
        {
            return hr;
        }

        // -------------------------------------------------------------------------
        // Move application selection to the end of the committed Bangla text.
        // -------------------------------------------------------------------------
        TF_SELECTION selection{};
        selection.range = caret_range.Get();
        selection.style.ase = TF_AE_NONE;
        selection.style.fInterimChar = FALSE;

        hr = context->SetSelection(edit_cookie, 1, &selection);
        if (FAILED(hr))
        {
            return hr;
        }

        active_context_ = context;
        ODRI_LOG_INFO("tsf composition update complete");

        return S_OK;
    }

    // =============================================================================
    // End current Odri word
    // =============================================================================
    HRESULT OdriTextService::EndComposition(ITfContext *context)
    {
        (void)context;
        //
        // There is no ITfComposition anymore.
        //
        // The output is already ordinary committed text.
        //
        ODRI_LOG_INFO("end composition: releasing owned range");
        ResetOdriState();
        return S_OK;
    }

    // =============================================================================
    // Request end operation
    // =============================================================================
    HRESULT OdriTextService::DoCompositionEnd(ITfContext *context, TfEditCookie edit_cookie)
    {
        ODRI_LOG_INFO("composition end cookie=" + Hex(static_cast<unsigned long>(edit_cookie)));
        if (!context)
        {
            return E_INVALIDARG;
        }

        //
        // Nothing needs to be committed because the text was already committed
        // on every key.
        //
        ResetOdriState();
        ODRI_LOG_INFO("composition state released");

        return S_OK;
    }

    // =============================================================================
    // Backspace
    // =============================================================================
    // HRESULT OdriTextService::DoBackspace(ITfContext *context, TfEditCookie edit_cookie)
    // {
    //     if (!context)
    //     {
    //         return E_INVALIDARG;
    //     }

    //     if (!latin_buffer_.empty() && IsOwnedRangeAtSelection(context, edit_cookie))
    //     {
    //         latin_buffer_.pop_back();
    //         return DoCompositionUpdate(context, edit_cookie, latin_buffer_.empty() ? '\0' : latin_buffer_.back());
    //     }

    //     ResetOdriState();
    //     return DoDeleteSelection(context, edit_cookie);
    // }

    // =============================================================================
    // Delete current selection
    // (or, if collapsed, the character before it)
    // =============================================================================
    HRESULT OdriTextService::DoDeleteSelection(ITfContext *context, TfEditCookie edit_cookie)
    {
        if (!context)
        {
            return E_INVALIDARG;
        }

        TF_SELECTION selection{};
        ULONG fetched = 0;

        HRESULT hr = context->GetSelection(edit_cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched);
        if (FAILED(hr))
        {
            return hr;
        }

        if (fetched == 0 || !selection.range)
        {
            return S_OK;
        }

        // GetSelection AddRefs the range it returns.
        // Attach that reference to a ComPtr so it is always released.
        Microsoft::WRL::ComPtr<ITfRange> range;
        range.Attach(selection.range);

        BOOL is_empty = FALSE;
        hr = range->IsEmpty(edit_cookie, &is_empty);
        if (FAILED(hr))
        {
            return hr;
        }

        if (is_empty)
        {
            // ---------------------------------------------------------------------
            // Nothing highlighted.
            //
            // Plain backspace deletes the character immediately before the caret.
            //
            // Move the START of the collapsed range one UTF-16 code unit left.
            //
            // SDK 10.0.26100.0 also returns the number of characters moved.
            // ---------------------------------------------------------------------
            LONG moved = 0;
            hr = range->ShiftStart(edit_cookie, -1, &moved, nullptr);
            if (FAILED(hr))
            {
                return hr;
            }

            // If the caret was already at the beginning of the context,
            // ShiftStart(-1) leaves the range empty.
            BOOL still_empty = FALSE;
            hr = range->IsEmpty(edit_cookie, &still_empty);
            if (FAILED(hr))
            {
                return hr;
            }

            if (still_empty)
            {
                return S_OK;
            }
        }

        // Delete the selected/extended range.
        hr = range->SetText(edit_cookie, 0, L"", 0);
        if (FAILED(hr))
        {
            return hr;
        }

        ResetOdriState();
        return S_OK;
    }

} // namespace odri_windows