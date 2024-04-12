#include "pyson.hpp"

PysonValue::~PysonValue() {
    
    // because all variants start with the `type` field, no undefined behavior
    // (I think)
    switch (this->int_value.type) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            break;
        case PysonType::PysonStr:
            this->str_value.value.~basic_string();
            break;
        case PysonType::PysonStrList:
            this->str_list_value.value.~vector();
            break;
    }
    
}

std::ostream& operator<< (std::ostream& o, const PysonValue& val) {
    switch (val.int_value.type) {
        case PysonValue::PysonType::PysonInt: o << val.int_value.value;
        case PysonValue::PysonType::PysonFloat: o << val.float_value.value;
        case PysonValue::PysonType::PysonStr: o << val.str_value.value;
        case PysonValue::PysonType::PysonStrList: o << val.str_list_value.value;
    }
    return o;
}