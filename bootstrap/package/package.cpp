#include "package/package.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <filesystem>

namespace colgm {

void package_manager::generate_search_order() {
    search_order.clear();
    if (!library_path.empty()) {
        search_order.push_back(library_path);
    }
    search_order.push_back(".");
    const auto env = getenv("PATH");
#ifdef _WIN32
    const char sep = ';';
#else
    const char sep = ':';
#endif

    std::string path = "";
    for (auto i = env; *i; i++) {
        if (*i == sep) {
            search_order.push_back(path);
            path = "";
            continue;
        }
        path += *i;
    }
    if (!path.empty()) {
        search_order.push_back(path);
    }
}

std::string package_manager::find(const std::string& module_name,
                                  const std::string& file_name) {
    for (const auto& i : search_order) {
#ifdef _WIN32
        const char sep = '\\';
#else
        const char sep = '/';
#endif
        const auto path = i + sep + file_name;
        if (!std::filesystem::exists(path)) {
            continue;
        }

        const auto absolute_path = std::filesystem::absolute(path).string();

        file_to_module.insert({absolute_path, module_name});
        module_to_file.insert({module_name, absolute_path});
        analyse_status_map.insert({absolute_path, status::not_used});
        return absolute_path;
    }
    return "";
}

}