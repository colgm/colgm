#include "code/value.h"
#include "code/sir.h"

namespace colgm {

void value::dump(std::ostream& out) const {
    out << content << " : " << resolve_type;
}

value_t value::to_value_t() const  {
    return value_t::variable(content);
}

}