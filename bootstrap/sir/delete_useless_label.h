#pragma once

#include "sir/pass_manager.h"

#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm {

class delete_useless_label: public sir_pass {
private:
    u64 removed_count;

private:
    void add_count(std::unordered_map<std::string, u64>&, const std::string&);
    void delete_label(sir_block*);

public:
    delete_useless_label(): sir_pass(), removed_count(0) {}
    ~delete_useless_label() override = default;
    std::string name() override {
        return "delete useless label";
    }
    std::string info() override {
        std::stringstream ss;
        ss << "removed " << removed_count << " instruction";
        if (removed_count > 1) {
            ss << "s";
        }
        return ss.str();
    }
    bool run(sir_context*) override;
};

}