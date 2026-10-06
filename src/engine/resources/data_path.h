#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
//#include <iostream>
#include <fstream>

using namespace std;
namespace openAITD {

  class DataPath {
  public:
      static bool FileExists(const string path) {
          std::ifstream file(path);
          return file.good();
      }

      static string GetFile(const string path) {
        string checkPath = "./moddata/" + path;
        if (FileExists(checkPath)) return checkPath;
        checkPath = "./newdata/" + path;
        if (FileExists(checkPath)) return checkPath;
        checkPath = "./data/" + path;
        if (FileExists(checkPath)) return checkPath;
        return "";
      }

      // Scans and combines subdirectories within data/<path>, newdata/<path>, and
      // moddata/<path>. Returns the names of subdirectories without duplicates, in the order
      // of mod priority (moddata -> newdata -> data), as in GetFile().
      static vector<string> ListDirs(const string path) {
        static const char* roots[] = { "./moddata/", "./newdata/", "./data/" };
        vector<string> result;
        std::error_code ec;
        for (const char* root : roots) {
          std::filesystem::path dir = std::filesystem::path(root) / path;
          if (!std::filesystem::is_directory(dir, ec)) continue;
          for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (!entry.is_directory()) continue;
            string name = entry.path().filename().string();
            if (std::find(result.begin(), result.end(), name) == result.end()) {
              result.push_back(name);
            }
          }
        }
        return result;
      }
  };

}