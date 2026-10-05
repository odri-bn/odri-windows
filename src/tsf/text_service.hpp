#pragma once

#include <msctf.h>
#include <windows.h>
#include <string>
#include <wrl/client.h>
#include <vector>

#include "core/engine_host.hpp"
#include "tsf/edit_session.hpp"

namespace odri_windows
{

    class CompositionEditSession;

    class OdriTextService
        : public ITfTextInputProcessorEx,
          public ITfKeyEventSink
    {
    public:
        OdriTextService();

        // -------------------------------------------------------------------------
        // IUnknown
        // -------------------------------------------------------------------------

        STDMETHODIMP QueryInterface(
            REFIID riid,
            void **ppv) override;

        STDMETHODIMP_(ULONG)
        AddRef() override;

        STDMETHODIMP_(ULONG)
        Release() override;

        // -------------------------------------------------------------------------
        // ITfTextInputProcessor
        // -------------------------------------------------------------------------

        STDMETHODIMP Activate(
            ITfThreadMgr *thread_mgr,
            TfClientId client_id) override;

        STDMETHODIMP Deactivate() override;

        // -------------------------------------------------------------------------
        // ITfTextInputProcessorEx
        // -------------------------------------------------------------------------

        STDMETHODIMP ActivateEx(
            ITfThreadMgr *thread_mgr,
            TfClientId client_id,
            DWORD flags) override;

        // -------------------------------------------------------------------------
        // ITfKeyEventSink
        // -------------------------------------------------------------------------

        STDMETHODIMP OnSetFocus(
            BOOL foreground) override;

        STDMETHODIMP OnTestKeyDown(
            ITfContext *context,
            WPARAM wParam,
            LPARAM lParam,
            BOOL *eaten) override;

        STDMETHODIMP OnTestKeyUp(
            ITfContext *context,
            WPARAM wParam,
            LPARAM lParam,
            BOOL *eaten) override;

        STDMETHODIMP OnKeyDown(
            ITfContext *context,
            WPARAM wParam,
            LPARAM lParam,
            BOOL *eaten) override;

        STDMETHODIMP OnKeyUp(
            ITfContext *context,
            WPARAM wParam,
            LPARAM lParam,
            BOOL *eaten) override;

        STDMETHODIMP OnPreservedKey(
            ITfContext *context,
            REFGUID rguid,
            BOOL *eaten) override;

        // -------------------------------------------------------------------------
        // Edit-session entry points
        // -------------------------------------------------------------------------

        HRESULT DoCompositionUpdate(
            ITfContext *context,
            TfEditCookie edit_cookie,
            char latin);

        HRESULT DoCompositionEnd(
            ITfContext *context,
            TfEditCookie edit_cookie);

        HRESULT DoBackspace(
            ITfContext *context,
            TfEditCookie edit_cookie);

        HRESULT DoDeleteSelection(
            ITfContext *context,
            TfEditCookie edit_cookie);

        HRESULT EndComposition(
            ITfContext *context);

    private:
        ~OdriTextService();

        HRESULT AttachThreadManager(
            ITfThreadMgr *thread_mgr,
            TfClientId client_id,
            DWORD flags);

        void DetachThreadManager();

        // Requests a single ITfEditSession for the given operation. All
        // per-keystroke work (transliteration, range updates, deletion)
        // happens inside the resulting DoEditSession callback, so we only
        // ever pay for one RequestEditSession + one engine call per key.
        HRESULT RunEditSession(
            ITfContext *context,
            CompositionEditOperation operation,
            char latin = '\0');

        // -------------------------------------------------------------------------
        // Odri state
        // -------------------------------------------------------------------------

        void ResetOdriState();

        bool IsOwnedRangeAtSelection(
            ITfContext *context,
            TfEditCookie edit_cookie) const;

        LONG ref_count_;

        // -------------------------------------------------------------------------
        // TSF state
        // -------------------------------------------------------------------------

        Microsoft::WRL::ComPtr<ITfThreadMgr> thread_mgr_;
        Microsoft::WRL::ComPtr<ITfKeystrokeMgr> keystroke_mgr_;

        TfClientId client_id_ = TF_CLIENTID_NULL;
        DWORD activate_flags_ = 0;

        // -------------------------------------------------------------------------
        // Odri core
        // -------------------------------------------------------------------------

        EngineHost engine_;

        // -------------------------------------------------------------------------
        // Current Odri state
        // -------------------------------------------------------------------------

        // The Latin input that produced the current Bangla output.
        std::string latin_buffer_;

        // Current Bangla output from Odri.
        std::wstring composition_text_;

        // Last committed Bangla text used for incremental TSF diffs.
        std::wstring previous_text_;

        // unit
        std::vector<EngineUnit> previous_units_;

        // Context containing our committed Odri text.
        Microsoft::WRL::ComPtr<ITfContext> active_context_;

        // Range containing the committed Bangla text currently owned by Odri.
        //
        // Unlike ITfComposition, this is ordinary committed document text.
        // There is therefore no TSF composition underline.
        Microsoft::WRL::ComPtr<ITfRange> owned_range_;

        friend class CompositionEditSession;
    };

} // namespace odri_windows