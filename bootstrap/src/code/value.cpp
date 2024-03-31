#include "code/value.h"

namespace colgm {

void value::dump(std::ostream& out) const {
    out << content << " : " << resolve_type;
}

}