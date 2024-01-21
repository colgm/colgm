#include "package/package.h"

#include <cstring>
#include <sstream>
#include <filesystem>

namespace colgm {

void package_manager::dump_package_core(colgm_package* p,
                                        const std::string& indent) {
    for(const auto& i : p->sub_pack) {
        std::clog << indent << "package: " << i.first << ":\n";
        dump_package_core(i.second, indent + "  ");
    }
    for(const auto& i : p->sub_mod) {
        std::clog << indent << "module: " << i.first << "\n";
    }
}

void package_manager::dump_packages() {
    dump_package_core(&root_package, "");
}

void package_manager::add_new_file(const std::filesystem::path& fp) {
    std::vector<std::filesystem::path> parent_path_split;
    const auto parent = fp.parent_path().string();
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
        if (!pkg->sub_pack.count(d.string())) {
            pkg->add_new_package(d.string());
            pkg = pkg->sub_pack.at(d.string());
        }
    }
    if (!pkg->sub_mod.count(fp.stem().string())) {
        pkg->add_new_module(fp.stem().string());
    }
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
                add_new_file(std::filesystem::relative(i.path(), directory));
            }
        }
    }
    return err;
}

}