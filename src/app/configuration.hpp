// src/app/configuration.hpp
//
// Every identity constant of the text service lives here: the CLSID, the
// language profile GUID, the language the profile is registered under, and the
// display strings. Nothing else in the project should hardcode a GUID.
#pragma once

#include <windows.h>

#include <string>

namespace odri_windows
{

    // {6D27EED4-8B19-4F29-9046-720F559941E9}
    extern const CLSID kOdriTextServiceClsid;

    // {DB2F1C82-2B00-4DCF-B81C-90CF10527D6D}
    extern const GUID kOdriProfileGuid;

    // The substitute HKL (QWERTY) that the text service registers for its profile.
    constexpr UINT_PTR kOdriSubstituteHkl = 0x00000409;

    // English (United States). Odri uses the US QWERTY keyboard as its
    // physical input basis and transliterates Latin input into Bangla.
    constexpr LANGID kOdriLangId =
        MAKELANGID(LANG_BANGLA, SUBLANG_BANGLA_BANGLADESH);

    constexpr wchar_t kTextServiceDescription[] = L"Odri (Bangla Phonetic)";
    constexpr wchar_t kProfileDescription[] = L"Odri Phonetic";
    constexpr wchar_t kClsidRegistryDescription[] = L"Odri Bangla Phonetic Text Service";

    // Resolves the odri-core data directory (vowels.json, rules.json, ...).
    //
    // The working directory of a process that loads a text service DLL is the host
    // application's, which is meaningless to us, so resolution is anchored to the
    // DLL's own location and to an explicit registry/environment override instead.
    //
    // Order:
    //   1. HKCU\Software\Odri\DataDir           (per-user override)
    //   2. %ODRI_DATA%                          (developer override)
    //   3. <dll directory>\data                   (normal install layout)
    //   4. <dll directory>\..\data                (build tree layout)
    //   5. <dll directory>\..\..\data
    //
    // Returns an empty string when no candidate contains vowels.json.
    std::wstring ResolveDataDirectory();

} // namespace odri_windows
