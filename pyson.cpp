#include "pyson.hpp"
#include <iterator>
#include <cstring>
#include <sstream>

// PysonValue destructor
PysonValue::~PysonValue() noexcept {
    
    switch (this->get_type()) {
        
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            break;
        
        case PysonType::PysonStr:
            this->value.str_value.~basic_string();
            break;
        
        case PysonType::PysonStrList:
            this->value.str_list_value.~vector();
            break;
        
    }
    
}

// makes a PysonValue printable
std::ostream& operator<< (std::ostream& o, const PysonValue& val) {
    
    o << val.get_type_cstring();
    o << ':';
    o << val.value_as_string();
    
    return o;
}

// PysonValue copy constructor
PysonValue::PysonValue(const PysonValue& other) : value(0) {

    switch (other.get_type()) {

        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            this->value.str_value = other.value.str_value;
            break;

        case PysonType::PysonStrList:
            this->value.str_list_value = other.value.str_list_value;

    }

}
// PysonValue move constructor
PysonValue::PysonValue(PysonValue&& other) : value(0) {

    // (same code as copy constructor)

    switch (other.get_type()) {

        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            this->value.str_value = other.value.str_value;
            break;

        case PysonType::PysonStrList:
            this->value.str_list_value = other.value.str_list_value;

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
            this->value.str_value = other.value.str_value;
            break;

        case PysonType::PysonStrList:
            this->value.str_list_value = other.value.str_list_value;

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
            this->value.str_value = other.value.str_value;
            break;

        case PysonType::PysonStrList:
            this->value.str_list_value = other.value.str_list_value;

    }

    return *this;
    
}

// Returns "int", "float", "str", or "list"
const char *PysonValue::get_type_cstring() const noexcept {
    switch (this->get_type()) {
        case PysonType::PysonInt: return "int";
        case PysonType::PysonFloat: return "float";
        case PysonType::PysonStr: return "str";
        case PysonType::PysonStrList: return "list";
    }
}

// Turn the PysonValue's value into a pyson-formatted string
std::string PysonValue::value_as_string() const noexcept {
    switch (this->get_type()) {
        case PysonType::PysonInt: return std::to_string(this->value.int_value);
        case PysonType::PysonFloat: return std::to_string(this->value.float_value);
        case PysonType::PysonStr: return this->value.str_value;
        case PysonType::PysonStrList: {
            std::ostringstream pyson_list;
            std::copy(
                this->value.str_list_value.begin(),
                this->value.str_list_value.end(),
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

// Create a PysonValue from a list in the pyson format
PysonValue PysonValue::from_pyson_list(std::string pyson_list) {
    PysonValue value(std::vector<std::string>{});
    std::string current_token{};
    for (char c : pyson_list) {
        current_token.push_back(c);
        size_t size = current_token.length();
        if (size >= 3 && current_token.substr(size-4, 3) == "(*)") {
            value.value.str_list_value.push_back(current_token.substr(0, size-4));
            current_token.clear();
        }
    }
    return value;
}

// Output a NamedPysonValue in the pyson format
std::ostream& operator<< (std::ostream& o, NamedPysonValue& v) {
    o << v.name << ':' << v.value;
    return o;
}

// Read a pyson-formatted line into a NamedPysonValue
bool operator>> (std::istream& i, NamedPysonValue& v) {
    
    std::string current_token{};
    
    std::getline(i, current_token, ':');
    for (char c : current_token) if (c == '\n') return false;
    v.change_name(current_token);

    std::getline(i, current_token, ':');
    if (current_token == "int") v.value.type = PysonType::PysonFloat;
    else if (current_token == "float") v.value.type = PysonType::PysonFloat;
    else if (current_token == "str") v.value.type = PysonType::PysonStr;
    else if (current_token == "list") v.value.type = PysonType::PysonStrList;
    else return false;

    std::getline(i, current_token);
    switch (v.value.get_type()) {
        case PysonType::PysonInt:
            try { v.value = PysonValue(std::stoi(current_token)); break; }
            catch (...) { return false; }
        case PysonType::PysonFloat:
            try { v.value = PysonValue(std::stod(current_token)); break; }
            catch(...) { return false; }
        case PysonType::PysonStr: v.value = PysonValue(current_token); break;
        case PysonType::PysonStrList: v.value = PysonValue::from_pyson_list(current_token); break;
    }

    return true;
    
}