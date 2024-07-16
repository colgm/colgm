#pragma once

#include "report.h"
#include "sema/symbol.h"

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace colgm {

struct colgm_module {
    std::string file_name;
    std::unordered_map<std::string, colgm_enum> enums;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
};

struct colgm_package {
    std::string package_name;
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
        sub_pack.at(name)->package_name = name;
    }
    void add_new_module(const std::string& mn, const std::string& fn) {
        sub_mod.insert({mn, new colgm_module});
        sub_mod.at(mn)->file_name = fn;
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

public:
    enum class status {
        not_used,
        not_exist,
        analysed,
        analysing
    };

private:
    std::unordered_map<std::string, std::string> file_to_module;
    std::unordered_map<std::string, std::string> module_to_file;
    std::unordered_map<std::string, status> analyse_status_map;

    void recursive_dump_modules(colgm_package*, const std::string&);
    void recursive_dump_modules_root();
    void add_new_file(const std::filesystem::path&, const std::filesystem::path&);
    std::string replace_string(const std::string&);

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
    const auto get_analyse_status(const std::string& file_name) const {
        if (analyse_status_map.count(file_name)) {
            return analyse_status_map.at(file_name);
        }
        return status::not_exist;
    }
    void set_analyse_status(const std::string& file_name, status as) {
        if (analyse_status_map.count(file_name)) {
            analyse_status_map.at(file_name) = as;
        }
    }
    const error& scan(const std::string&);
};

}