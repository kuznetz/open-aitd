#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iostream>

namespace openAITD {

    using namespace ::std;
    using json = nlohmann::json;

    class NameCollection {
    public:
        NameCollection(const std::string& prefix) {
            this->prefix = prefix;
        }

        void load(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open file: " + filename);
            }
            json j;
            try {
                file >> j;
            } catch (const json::parse_error& e) {
                throw std::runtime_error("JSON parse error: " + std::string(e.what()));
            }
            if (!j.is_object()) {
                throw std::runtime_error("Root JSON element must be an object");
            }

            _names.clear();
            for (auto& [key, value] : j.items()) {
                if (!value.is_string()) {
                    throw std::runtime_error("Value for key '" + key + "' is not a string");
                }
                try {
                    int id = std::stoi(key);
                    _names[id] = value.get<std::string>();
                } catch (const std::invalid_argument&) {
                    throw std::runtime_error("Key '" + key + "' is not a valid integer");
                } catch (const std::out_of_range&) {
                    throw std::runtime_error("Key '" + key + "' is out of integer range");
                }
            }
        }

        bool has(int id) const {
            return _names.find(id) != _names.end();
        }

        std::string getName(int id) const {
            auto it = _names.find(id);
            if (it == _names.end()) {
                return prefix + std::to_string(id);
            }
            return it->second;
        }

        std::string operator[](int id) const {
            return getName(id);
        }

    private:
        std::string prefix;
        std::unordered_map<int, std::string> _names;
    };

}