#include "pyson.hpp"
#include <iterator>
#include <cstring>

PysonValue::~PysonValue() {
    
    // because all variants start with a `type` field
    // with type PysonType, no undefined behavior
    // same goes for all the other times this is used
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

// makes a PysonValue printable
std::ostream& operator<< (std::ostream& o, const PysonValue& val) {
    
    switch (val.int_value.type) {
        
        case PysonValue::PysonType::PysonInt: o << val.int_value.value; break;
        case PysonValue::PysonType::PysonFloat: o << val.float_value.value; break;
        case PysonValue::PysonType::PysonStr: o << val.str_value.value; break;
        
        case PysonValue::PysonType::PysonStrList: {
            o << "[\n";
            for (const std::string& str : val.str_list_value.value) { o << "\t" << str << "," << "\n"; }
            o << "]";
            break;
        }
        
    }
    
    return o;
}

// PysonValue copy constructor
PysonValue::PysonValue(const PysonValue& other) {

    switch (other.int_value.type) {

        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            this->str_value.value = other.str_value.value;
            break;

        case PysonType::PysonStrList:
            this->str_list_value.value = other.str_list_value.value;

    }

}

// PysonValue move constructor
PysonValue::PysonValue(PysonValue&& other) {

    // (same code as copy constructor)

    switch (other.int_value.type) {

        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            this->str_value.value = other.str_value.value;
            break;

        case PysonType::PysonStrList:
            this->str_list_value.value = other.str_list_value.value;

    }

}

PysonValue& PysonValue::operator= (const PysonValue& other) {
    
    // (same code as copy constructor other than `return *this` at the end)

    switch (other.int_value.type) {

        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            this->str_value.value = other.str_value.value;
            break;

        case PysonType::PysonStrList:
            this->str_list_value.value = other.str_list_value.value;

    }

    return *this;
    
}

PysonValue& PysonValue::operator= (PysonValue&& other) {

    // (same code as copy constructor other than `return *this` at the end)

    switch (other.int_value.type) {

        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            this->str_value.value = other.str_value.value;
            break;

        case PysonType::PysonStrList:
            this->str_list_value.value = other.str_list_value.value;

    }

    return *this;
    
}