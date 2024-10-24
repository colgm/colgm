#include "mir/mir.h"
#include "mir/visitor.h"

#include <iomanip>
#include <sstream>
#include <cstring>

namespace colgm::mir {

std::string to_hex(mir* num) {
    std::stringstream ss;
    ss << "0x" << std::hex << u64(num) << std::dec << "";
    return ss.str();
}

void mir::accept(visitor*) {}

void mir_block::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " block (";
    if (content.empty()) {
        os << ")\n";
        return;
    }

    os << "\n";
    for(auto i : content) {
        i->dump(indent + " ", os);
    }
    os << indent << ")\n";
}

void mir_block::accept(visitor* v) {
    v->visit_mir_block(this);
}

void mir_unary::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "] unary [";
    switch(opr) {
        case opr_kind::neg: os << "-"; break;
        case opr_kind::bnot: os << "~"; break;
        case opr_kind::lnot: os << "!"; break;
    }
    os << "] (\n";
    value->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_unary::accept(visitor* v) {
    v->visit_mir_unary(this);
}

void mir_binary::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "] binary [";
    switch(opr) {
        case opr_kind::add: os << "+"; break;
        case opr_kind::sub: os << "-"; break;
        case opr_kind::mult: os << "*"; break;
        case opr_kind::div: os << "/"; break;
        case opr_kind::rem: os << "%"; break;
        case opr_kind::cmpeq: os << "=="; break;
        case opr_kind::cmpneq: os << "!="; break;
        case opr_kind::less: os << "<"; break;
        case opr_kind::leq: os << "<="; break;
        case opr_kind::grt: os << ">"; break;
        case opr_kind::geq: os << ">="; break;
        case opr_kind::cmpand: os << "&&"; break;
        case opr_kind::cmpor: os << "||"; break;
        case opr_kind::band: os << "&"; break;
        case opr_kind::bor: os << "|"; break;
        case opr_kind::bxor: os << "^"; break;
    }
    os << "] (\n";
    left->dump(indent + " ", os);
    right->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_binary::accept(visitor* v) {
    v->visit_mir_binary(this);
}

void mir_type_convert::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [target:" << target_type << "] convert (\n";
    source->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_type_convert::accept(visitor* v) {
    v->visit_mir_type_convert(this);
}

void mir_nil::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " " << "[" << resolve_type << "] nil\n";
}

void mir_nil::accept(visitor* v) {
    v->visit_mir_nil(this);
}

void mir_number::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this);
    os << " [" << resolve_type << "] number: " << literal << "\n";
}

void mir_number::accept(visitor* v) {
    v->visit_mir_number(this);
}

void mir_string::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "] string: \"";
    os << llvm_raw_string(literal) << "\"\n";
}

void mir_string::accept(visitor* v) {
    v->visit_mir_string(this);
}

void mir_char::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "]";
    os << "[ascii(dec):" << int(literal) << "]";
    os << " char: \'\\";
    os << std::hex << std::setw(2) << std::setfill('0') << int(literal);
    os << std::dec << "\'\n";
}

void mir_char::accept(visitor* v) {
    v->visit_mir_char(this);
}

void mir_bool::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "]";
    os << " bool: " << (literal? "true":"false") << "\n";
}

void mir_bool::accept(visitor* v) {
    v->visit_mir_bool(this);
}

void mir_array::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << size << "x";
    os << resolve_type.get_ref_copy() << "] array\n";
}

void mir_array::accept(visitor* v) {
    v->visit_mir_array(this);
}

mir_struct_init::~mir_struct_init() {
    for(const auto& i : fields) {
        delete i.content;
    }
}

void mir_struct_init::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "] struct (\n";
    for(const auto& i : fields) {
        os << indent << "  [" << i.resolve_type << "] " << i.name << " : (\n";
        i.content->dump(indent + "    ", os);
        os << indent << "  )\n";
    };
    os << indent << ")\n";
}

void mir_struct_init::accept(visitor* v) {
    v->visit_mir_struct_init(this);
}

void mir_call::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "] call (\n";
    for(auto i : content->get_content()) {
        i->dump(indent + " ", os);
    }
    os << indent << ")\n";
}

void mir_call::accept(visitor* v) {
    v->visit_mir_call(this);
}

void mir_call_id::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " ";
    if (resolve_type.is_global) {
        os << "[global]";
    } else {
        os << "[real:" << resolve_type.get_pointer_copy() << "]";
    }
    os << " identifier: " << name << "\n";
}

void mir_call_id::accept(visitor* v) {
    v->visit_mir_call_id(this);
}

void mir_call_index::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "] call index (\n";
    index->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_call_index::accept(visitor* v) {
    v->visit_mir_call_index(this);
}

void mir_call_func::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "]";
    os << " call func (\n";
    args->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_call_func::accept(visitor* v) {
    v->visit_mir_call_func(this);
}

void mir_get_field::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this);
    os << " [real:" << resolve_type.get_pointer_copy() << "]";
    os << " get field: " << name << "\n";
}

void mir_get_field::accept(visitor* v) {
    v->visit_mir_get_field(this);
}

void mir_ptr_get_field::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this);
    os << " [real:" << resolve_type.get_pointer_copy() << "]";
    os << " pointer get field: " << name << "\n";
}

void mir_ptr_get_field::accept(visitor* v) {
    v->visit_mir_ptr_get_field(this);
}

void mir_get_path::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "]";
    os << (resolve_type.is_global? "[global]":"[instance]");
    os << " call path: " << name << "\n";
}

void mir_get_path::accept(visitor* v) {
    v->visit_mir_get_path(this);
}

void mir_define::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " [" << resolve_type << "]";
    os << " definition: " << name << " (\n";
    for(auto i : init_value->get_content()) {
        i->dump(indent + " ", os);
    }
    os << indent << ")\n";
}

void mir_define::accept(visitor* v) {
    v->visit_mir_define(this);
}

void mir_assign::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " assignment [";
    switch(opr) {
       case opr_kind::eq: os << "="; break;
       case opr_kind::addeq: os << "+="; break;
       case opr_kind::subeq: os << "-="; break;
       case opr_kind::multeq: os << "*="; break;
       case opr_kind::diveq: os << "/="; break;
       case opr_kind::remeq: os << "%="; break;
       case opr_kind::andeq: os << "&="; break;
       case opr_kind::xoreq: os << "^="; break;
       case opr_kind::oreq: os << "|="; break;
    }
    os << "] (\n";
    left->dump(indent + " ", os);
    right->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_assign::accept(visitor* v) {
    v->visit_mir_assign(this);
}

void mir_if::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " " << (condition? "if":"else") << " (\n";
    if (condition) {
        condition->dump(indent + " ", os);
    }
    content->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_if::accept(visitor* v) {
    v->visit_mir_if(this);
}

void mir_branch::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " branch (\n";
    for(auto i : branch) {
        i->dump(indent + " ", os);
    }
    os << indent << ")\n";
}

void mir_branch::accept(visitor* v) {
    v->visit_mir_branch(this);
}

void mir_break::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " break\n";
}

void mir_break::accept(visitor* v) {
    v->visit_mir_break(this);
}

void mir_continue::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " continue\n";
}

void mir_continue::accept(visitor* v) {
    v->visit_mir_continue(this);
}

void mir_loop::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " loop (\n";
    condition->dump(indent + " ", os);
    content->dump(indent + " ", os);
    if (update) {
        update->dump(indent + " ", os);
    }
    os << indent << ")\n";
}

void mir_loop::accept(visitor* v) {
    v->visit_mir_loop(this);
}

void mir_return::dump(const std::string& indent, std::ostream& os) {
    os << indent << to_hex(this) << " return (\n";
    value->dump(indent + " ", os);
    os << indent << ")\n";
}

void mir_return::accept(visitor* v) {
    v->visit_mir_return(this);
}

}