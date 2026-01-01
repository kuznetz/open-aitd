#pragma once
#include <string>
#include <string>
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
  };

}