#pragma once

#include "sir/sir.h"
#include "sir/context.h"

#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

class sir_pass {
public:
    virtual ~sir_pass() = default;
    virtual std::string name() = 0;
    virtual std::string info() = 0;
    virtual bool run(sir_context*) = 0;
};

class sir_pass_manager {
private:
    std::vector<sir_pass*> passes;

public:
    ~sir_pass_manager();
    bool execute(sir_context*, bool);
};

}