#include "ast/ast.h"

#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

struct colgm_struct {
    std::string name;
    std::unordered_map<std::string, uint32_t> field;
};

class semantic {};

}