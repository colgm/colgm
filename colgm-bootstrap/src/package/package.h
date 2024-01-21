#pragma once

#include "err.h"
#include "sema/symbol.h"

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace colgm {

struct colgm_module {
    std::string file_name;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
};

struct colgm_package {
    std::unordered_map<std::string, colgm_package*> sub_pack;
    std::unordered_map<std::string, colgm_module*> sub_mod;

    ~colgm_package() {
        for(auto& i : sub_pack) {
            delete i.second;
        }
        for(auto& i : sub_mod) {
            delete i.second;
        }
    }
    void add_new_package(const std::string& name) {
        sub_pack.insert({name, new colgm_package});
    }
    void add_new_module(const std::string& name) {
        sub_mod.insert({name, new colgm_module});
    }
};

class package_manager {
private:
    error err;

private:
    colgm_package root_package;
    void dump_package_core(colgm_package*, const std::string&);

public:
    void dump_packages();

private:
    std::unordered_map<std::string, std::string> file_to_module;
    std::unordered_map<std::string, std::string> module_to_file;

    void add_new_module(const std::string& module_name, const std::string& file_name) {
        file_to_module.insert({file_name, module_name});
        module_to_file.insert({module_name, file_name});
    }
    void add_new_file(const std::filesystem::path&);

public:
    static package_manager* singleton() {
        static package_manager pm;
        return &pm;
    }
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
    const error& scan(const std::string&);
    
};

}