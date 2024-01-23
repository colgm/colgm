#include "package/package.h"

#include <cstring>
#include <sstream>
#include <filesystem>

namespace colgm {

void package_manager::dump_package_core(colgm_package* p,
                                        const std::string& indent) {
    for(const auto& i : p->sub_pack) {
        std::clog << indent << "package: " << i.second->package_name << ":\n";
        dump_package_core(i.second, indent + "  ");
    }
    for(const auto& i : p->sub_mod) {
        std::clog << indent << "module: " << i.first;
        std::clog << " -> " << i.second->file_name << "\n";
    }
}

void package_manager::dump_packages() {
    dump_package_core(&root_package, "");
    std::clog << "\n";
    for(const auto& i : module_to_file) {
        std::clog << i.first << " -> " << i.second << "\n";
    }
}

void package_manager::recursive_dump_modules(colgm_package* pkg,
                                             const std::string& path) {
    
    const auto parent_path = path.empty()? "":(path + "::");
    for(const auto& i : pkg->sub_pack) {
        recursive_dump_modules(i.second, parent_path + pkg->package_name);
    }
    for(const auto& i : pkg->sub_mod) {
        const auto mod_path = parent_path + pkg->package_name + "::" + i.first;
        file_to_module.insert({i.second->file_name, mod_path});
        module_to_file.insert({mod_path, i.second->file_name});
        analyse_status_map.insert({i.second->file_name, analyse_status::not_used});
    }
}

void package_manager::recursive_dump_modules_root() {
    for(const auto& i : root_package.sub_pack) {
        recursive_dump_modules(i.second, "");
    }
    for(const auto& i : root_package.sub_mod) {
        file_to_module.insert({i.second->file_name, i.first});
        module_to_file.insert({i.first, i.second->file_name});
        analyse_status_map.insert({i.second->file_name, analyse_status::not_used});
    }
}

void package_manager::add_new_file(const std::filesystem::path& canonical_path,
                                   const std::filesystem::path& real_path) {
    std::vector<std::filesystem::path> parent_path_split;
    const auto parent = canonical_path.parent_path().string();
    size_t last = 0;
    size_t sep = parent.find_first_of("/\\", last);
    while(sep!=std::string::npos) {
        parent_path_split.push_back(parent.substr(last, sep-last));
        last = sep+1;
        sep = parent.find_first_of("/\\", last);
    }
    if (last!=parent.length()) {
        parent_path_split.push_back(parent.substr(last, parent.length()-last));
    }
    auto pkg = &root_package;
    for(const auto& d : parent_path_split) {
        const auto pkg_name = replace_string(d.string());
        if (!pkg->sub_pack.count(pkg_name)) {
            pkg->add_new_package(pkg_name);
            pkg = pkg->sub_pack.at(pkg_name);
        } else {
            pkg = pkg->sub_pack.at(pkg_name);
        }
    }
    const auto mod_name = replace_string(canonical_path.stem().string());
    if (!pkg->sub_mod.count(mod_name)) {
        pkg->add_new_module(mod_name, real_path.string());
    }
}

std::string package_manager::replace_string(const std::string& src) {
    auto result = src;
    auto pos = result.find_first_of("-.", 0);
    while(pos!=std::string::npos) {
        result[pos] = '_';
        pos = result.find_first_of("-.", pos+1);
    }
    return result;
}

const error& package_manager::scan(const std::string& directory) {
    if (!std::filesystem::is_directory(directory)) {
        err.err("package",
            "\"" + directory + "\" does not exist or is not a directory."
        );
        return err;
    }
    for(auto i : std::filesystem::recursive_directory_iterator(directory)) {
        if (i.is_regular_file()) {
            if (i.path().extension()==".colgm") {
                add_new_file(std::filesystem::relative(i.path(), directory), i.path());
            }
        }
    }
    recursive_dump_modules_root();
    return err;
}

}