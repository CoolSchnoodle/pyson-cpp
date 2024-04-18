#include "pyson.hpp"
#include <iterator>
#include <cstring>
#include <sstream>

PysonValue::~PysonValue() noexcept {
    
    // because all variants start with a `type` field
    // with type PysonType, no undefined behavior
    // same goes for all the other times this is used
    switch (this->get_type()) {
        
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
    
    switch (val.get_type()) {
        
        case PysonValue::PysonType::PysonInt: o << val.int_value.value; break;
        case PysonValue::PysonType::PysonFloat: o << val.float_value.value; break;
        case PysonValue::PysonType::PysonStr: o << val.str_value.value; break;
        
        case PysonValue::PysonType::PysonStrList: {
            o << "[";
            for (const std::string& str : val.str_list_value.value) { o << str << ","; }
            o << "]";
            break;
        }
        
    }
    
    return o;
}

// PysonValue copy constructor
PysonValue::PysonValue(const PysonValue& other) {

    switch (other.get_type()) {

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

    switch (other.get_type()) {

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
// PysonValue copy assignment
PysonValue& PysonValue::operator= (const PysonValue& other) {
    
    // (same code as copy constructor other than `return *this` at the end)

    switch (other.get_type()) {

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
// PysonValue move assignment
PysonValue& PysonValue::operator= (PysonValue&& other) {

    // (same code as copy constructor other than `return *this` at the end)

    switch (other.get_type()) {

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

const char *PysonValue::get_type_cstring() const noexcept {
    switch (this->get_type()) {
        case PysonType::PysonInt: return "int";
        case PysonType::PysonFloat: return "float";
        case PysonType::PysonStr: return "str";
        case PysonType::PysonStrList: return "list";
    }
}

std::string PysonValue::value_as_string() const noexcept {
    switch (this->get_type()) {
        case PysonType::PysonInt: return std::to_string(this->int_value.value);
        case PysonType::PysonFloat: return std::to_string(this->float_value.value);
        case PysonType::PysonStr: return this->str_value.value;
        case PysonType::PysonStrList: {
            std::ostringstream pyson_list;
            std::copy(
                this->str_list_value.value.begin(),
                this->str_list_value.value.end(),
                std::ostream_iterator<std::string>(pyson_list, "(*)")
            );
            std::string str = pyson_list.str();
            str.pop_back();
            str.pop_back();
            str.pop_back();
            return str;
        }
    }
}

std::ostream& operator<< (std::ostream& o, NamedPysonValue& v) {
    o << v.name << ':' << v.value.get_type_cstring() << ':' << v.value.value_as_string();
    return o;
}