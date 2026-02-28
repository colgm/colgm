#pragma once

#include "report.h"
#include "sema/type.h"
#include "sema/func.h"
#include "sema/struct.h"
#include "sema/enum.h"
#include "sema/tagged_union.h"

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace colgm {

struct symbol_info {
    sym_kind kind;
    std::string loc_file;
    bool is_public = false;
};

struct colgm_module {
    std::string file_name;
    std::unordered_map<std::string, colgm_enum> enums;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_tagged_union> unions;
    std::unordered_map<std::string, colgm_func> functions;
    std::unordered_map<std::string, colgm_struct> generic_structs;
    std::unordered_map<std::string, colgm_func> generic_functions;

    // store global symbols used in current scope
    // both normal and generic symbols are stored
    // so if want to check if symbol exists, it's enough to just use this
    std::unordered_map<std::string, symbol_info> global_symbol;

    // only store generics in current scope
    // for type resolver to check if a symbol is generic
    std::unordered_map<std::string, symbol_info> generic_symbol;
};

class package_manager {
public:
    enum class status {
        not_used,
        not_exist,
        analysed,
        analysing
    };

private:
    std::string library_path = "";
    std::vector<std::string> search_order;
    std::unordered_map<std::string, std::string> file_to_module;
    std::unordered_map<std::string, std::string> module_to_file;
    std::unordered_map<std::string, status> analyse_status_map;

public:
    static package_manager* singleton() {
        static package_manager pm;
        return &pm;
    }
    void set_library_path(const std::string& path) {
        library_path = path;
    }
    void generate_search_order();
    void dump_search_order() const {
        for (const auto& s : search_order) {
            std::clog << "  " << s << std::endl;
        }
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
    std::string find(const std::string&, const std::string&);
};

}