#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace colgm {

class package_manager {
private:
    std::unordered_map<std::string, std::string> file_to_module;
    std::unordered_map<std::string, std::string> module_to_file;

public:
    const std::string& get_file_name(const std::string& module_name) const {
        static std::string null_name = "";
        if (module_to_file.count(module_name)) {
            return module_to_file.at(module_name);
        }
        return null_name;
    }
    const std::string& get_module_name(const std::string& file_name) const {
        static std::string null_name = "";
        if (file_to_module.count(file_name)) {
            return file_to_module.at(file_name);
        }
        return null_name;
    }
};

}