#include "error/span.h"

namespace colgm {

std::ostream& operator<<(std::ostream& out, const span& self) {
    if (self.file.size()>2 && self.file.substr(0, 2)=="./") {
        out << self.file.substr(2, self.file.size()-2);
    } else {
        out << self.file;
    }
    out << ":" << self.begin_line << ":" << self.begin_column + 1;
    return out;
}

}
