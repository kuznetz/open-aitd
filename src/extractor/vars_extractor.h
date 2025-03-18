#pragma once
#include "structs/int_types.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json_fwd.hpp>

using namespace std;
using json = nlohmann::json;

u8* loadFullFile(string path, int& size) {
    auto fHandle = fopen(path.c_str(), "rb");
    fseek(fHandle, 0, SEEK_END);
    size = ftell(fHandle);
    fseek(fHandle, 0, SEEK_SET);
    u8* outData = new u8[size];
    if (fread(outData, size, 1, fHandle) != 1)
        throw new exception("Error reading OBJETS.ITD");
    fclose(fHandle);
    return outData;
}

void extractVars(string dir, string josnTo) {
    json outJson = json::object();

    //VARS.ITD
    int dataBytes = 0;
    u8* vars = loadFullFile(dir + "/VARS.ITD", dataBytes);
    int varsCount = dataBytes / 2;
    json varsJson = json::object();
    for (int i = 0; i < varsCount; i++) {
        string s = string("v_") + to_string(i);
        varsJson[s] = READ_LE_S16(vars + (i*2));
    }
    delete vars;
    outJson["vars"] = varsJson;

    //DEFINES.ITD
    vars = loadFullFile(dir + "/DEFINES.ITD", dataBytes);
    varsCount = (dataBytes / 2);
    json cvarsJson = json::object();
    for (int i = 0; i < varsCount; i++) {
        string s = string("cv_") + to_string(i);
        cvarsJson[s] = READ_LE_S16(vars + (i * 2));
    }
    delete vars;
    outJson["cvars"] = cvarsJson;

    std::ofstream o(josnTo.c_str());
    o << std::setw(2) << outJson << std::endl;
}