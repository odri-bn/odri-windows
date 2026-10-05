#include "tsf/edit_session.hpp"

#include <string>

#include "tsf/text_service.hpp"
#include "util/log.hpp"

namespace okkhor_windows
{

    namespace
    {

        std::string Hex(unsigned long value)
        {
            char buffer[19] = {};

            std::snprintf(
                buffer,
                sizeof(buffer),
                "0x%08lX",
                value);

            return buffer;
        }

    } // namespace

    CompositionEditSession::CompositionEditSession(
        OkkhorTextService *service,
        ITfContext *context,
        CompositionEditOperation operation,
        char latin)
        : service_(service),
          context_(context),
          operation_(operation),
          latin_(latin)
    {
    }

    STDMETHODIMP CompositionEditSession::QueryInterface(
        REFIID riid,
        void **ppv)
    {
        if (!ppv)
            return E_INVALIDARG;

        *ppv = nullptr;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_ITfEditSession))
        {
            *ppv = static_cast<ITfEditSession *>(this);
        }

        if (!*ppv)
            return E_NOINTERFACE;

        AddRef();

        return S_OK;
    }

    STDMETHODIMP_(ULONG)
    CompositionEditSession::AddRef()
    {
        return static_cast<ULONG>(
            ::InterlockedIncrement(&ref_count_));
    }

    STDMETHODIMP_(ULONG)
    CompositionEditSession::Release()
    {
        const LONG remaining =
            ::InterlockedDecrement(&ref_count_);

        if (remaining == 0)
            delete this;

        return static_cast<ULONG>(remaining);
    }

    STDMETHODIMP CompositionEditSession::DoEditSession(
        TfEditCookie edit_cookie)
    {
        if (!service_ || !context_)
        {
            OKKHOR_LOG_ERROR("edit session missing service or context");
            return E_UNEXPECTED;
        }

        HRESULT hr = E_UNEXPECTED;

        switch (operation_)
        {
        case CompositionEditOperation::Update:

            hr =
                service_->DoCompositionUpdate(
                    context_.Get(),
                    edit_cookie,
                    latin_);

            break;

        // case CompositionEditOperation::Backspace:

        //     hr =
        //         service_->DoBackspace(
        //             context_.Get(),
        //             edit_cookie);

        //     break;

        case CompositionEditOperation::End:

            hr =
                service_->DoCompositionEnd(
                    context_.Get(),
                    edit_cookie);

            break;

        case CompositionEditOperation::DeleteSelection:

            hr =
                service_->DoDeleteSelection(
                    context_.Get(),
                    edit_cookie);

            break;

        default:

            OKKHOR_LOG_ERROR("edit session unknown operation");
            return E_UNEXPECTED;
        }

        if (FAILED(hr))
        {
            OKKHOR_LOG_ERROR("edit session operation failed hr=" +
                             Hex(static_cast<unsigned long>(hr)));
        }

        return hr;
    }

} // namespace okkhor_windows