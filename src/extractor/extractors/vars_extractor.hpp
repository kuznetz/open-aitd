#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

#include <nlohmann/json.hpp>
#include "../structs/int_types.h"

namespace AITDExtractor {

    using namespace std;
    using json = nlohmann::json;

    /**
     * Class for loading and exporting VARS.ITD and DEFINES.ITD files to JSON
     */
    class VarsExtractor {
    public:
        /**
         * Constructor
         * @param directory path to the directory containing VARS.ITD and DEFINES.ITD files
         */
        explicit VarsExtractor(const std::string& directory)
            : directory_(directory) {}

        /**
         * Loads VARS.ITD file into vars_ vector
         * @throws std::runtime_error if the file cannot be opened or read
         */
        void loadVars() {
            vars_ = loadFileValues("VARS.ITD");
        }

        /**
         * Loads DEFINES.ITD file into defines_ vector
         * @throws std::runtime_error if the file cannot be opened or read
         */
        void loadDefines() {
            defines_ = loadFileValues("DEFINES.ITD");
        }

        /**
         * Saves loaded data to a JSON file
         * @param jsonPath path to the output JSON file
         * @throws std::runtime_error if the file cannot be opened for writing
         */
        void saveToJson(const std::string& jsonPath) const {
            json outJson = json::object();

            // Convert vectors to JSON arrays
            json varsArray = json::array();
            for (int16_t v : vars_) {
                varsArray.push_back(v);
            }
            outJson["vars"] = varsArray;

            json definesArray = json::array();
            for (int16_t d : defines_) {
                definesArray.push_back(d);
            }
            outJson["cVars"] = definesArray;

            std::ofstream o(jsonPath);
            if (!o.is_open()) {
                throw std::runtime_error("Cannot open output JSON file: " + jsonPath);
            }
            o << std::setw(2) << outJson << std::endl;
        }

        const std::vector<int16_t>& getVars() const { return vars_; }
        const std::vector<int16_t>& getDefines() const { return defines_; }

    private:
        std::string directory_;
        std::vector<int16_t> vars_;
        std::vector<int16_t> defines_;

        /**
         * Helper function: loads a binary file and returns a vector of 16-bit values
         * @param filename file name (relative to the directory)
         * @return vector of int16_t values
         * @throws std::runtime_error on I/O errors or if size is not a multiple of 2
         */
        std::vector<int16_t> loadFileValues(const std::string& filename) const {
            std::string fullPath = directory_ + "/" + filename;

            std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open file: " + fullPath);
            }

            std::streamsize size = file.tellg();
            if (size % 2 != 0) {
                throw std::runtime_error("File size is not a multiple of 2: " + fullPath);
            }

            file.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(static_cast<size_t>(size));
            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                throw std::runtime_error("Error reading file: " + fullPath);
            }

            size_t count = static_cast<size_t>(size) / 2;
            std::vector<int16_t> result;
            result.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                result.push_back(READ_LE_S16(buffer.data() + i * 2));
            }
            return result;
        }
    };

}