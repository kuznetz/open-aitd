#include "platform.h"
#include <cstdlib>
#include <string>

std::string getSystemLanguageCode() {
    const char* lang = std::getenv("LANG");
    if (!lang) return "en";
    std::string s(lang);
    if (s.length() >= 2)
        return s.substr(0, 2);
    return "en";
}