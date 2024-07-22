#include "mir/mir2sir.h"

namespace colgm::mir {

void mir2sir::generate(const mir_context& mctx) {
    for(auto i : mctx.impls) {
        i->block->accept(this);
    }
}

}
