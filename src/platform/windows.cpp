#include "platform.h"
#include <windows.h>
#include <winnls.h>
#include <string>

std::string getSystemLanguageCode() {
    LANGID langId = GetUserDefaultUILanguage();
    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    if (LCIDToLocaleName(langId, name, LOCALE_NAME_MAX_LENGTH, 0) == 0)
        return "en";
    int size = WideCharToMultiByte(CP_UTF8, 0, name, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, name, -1, result.data(), size, nullptr, nullptr);
    return result.substr(0, 2);
}