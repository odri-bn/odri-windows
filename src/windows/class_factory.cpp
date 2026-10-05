// src/windows/class_factory.cpp
#include "windows/class_factory.hpp"

#include <new>

#include "tsf/text_service.hpp"
#include "util/log.hpp"
#include "windows/module.hpp"

namespace odri_windows {

OdriClassFactory::OdriClassFactory() : ref_count_(1) { ModuleAddRef(); }
OdriClassFactory::~OdriClassFactory() { ModuleRelease(); }

STDMETHODIMP OdriClassFactory::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_INVALIDARG;
    *ppv = nullptr;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
        *ppv = static_cast<IClassFactory*>(this);
    if (!*ppv) return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) OdriClassFactory::AddRef() {
    return static_cast<ULONG>(::InterlockedIncrement(&ref_count_));
}

STDMETHODIMP_(ULONG) OdriClassFactory::Release() {
    const LONG remaining = ::InterlockedDecrement(&ref_count_);
    if (remaining == 0) delete this;
    return static_cast<ULONG>(remaining);
}

STDMETHODIMP OdriClassFactory::CreateInstance(IUnknown* outer, REFIID riid, void** ppv) {
    if (!ppv) return E_INVALIDARG;
    *ppv = nullptr;
    if (outer) return CLASS_E_NOAGGREGATION;  // aggregation is not supported

    auto* service = new (std::nothrow) OdriTextService();
    if (!service) return E_OUTOFMEMORY;

    const HRESULT hr = service->QueryInterface(riid, ppv);
    service->Release();  // the caller now owns the only reference, or none
    return hr;
}

STDMETHODIMP OdriClassFactory::LockServer(BOOL lock) {
    if (lock) ModuleAddRef();
    else ModuleRelease();
    return S_OK;
}

}  // namespace odri_windows
