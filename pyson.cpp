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

PysonValue PysonValue::from_pyson_list(std::string pyson_list) {
    PysonValue value(std::vector<std::string>{});
    std::string current_token{};
    for (char c : pyson_list) {
        current_token.push_back(c);
        size_t size = current_token.length();
        if (size >= 3 && current_token.substr(size-4, 3) == "(*)") {
            value.str_list_value.value.push_back(current_token.substr(0, size-4));
            current_token.clear();
        }
    }
    return value;
}

std::ostream& operator<< (std::ostream& o, NamedPysonValue& v) {
    o << v.name << ':' << v.value.get_type_cstring() << ':' << v.value.value_as_string();
    return o;
}

bool operator>> (std::istream& i, NamedPysonValue& v) {
    
    std::string current_token{};
    
    std::getline(i, current_token, ':');
    for (char c : current_token) if (c == '\n') return false;
    v.change_name(current_token);

    std::getline(i, current_token, ':');
    if (current_token == "int") v.value.int_value.type = PysonValue::PysonType::PysonFloat;
    else if (current_token == "float") v.value.float_value.type = PysonValue::PysonType::PysonFloat;
    else if (current_token == "str") v.value.str_value.type = PysonValue::PysonType::PysonStr;
    else if (current_token == "list") v.value.str_list_value.type = PysonValue::PysonType::PysonStrList;
    else return false;

    std::getline(i, current_token);
    switch (v.value.get_type()) {
        case PysonValue::PysonType::PysonInt:
            try { v.value.int_value.value = std::stoi(current_token); break; }
            catch (...) { return false; }
        case PysonValue::PysonType::PysonFloat:
            try { v.value.float_value.value = std::stod(current_token); break; }
            catch(...) { return false; }
        case PysonValue::PysonType::PysonStr: v.value.str_value.value = current_token; break;
        case PysonValue::PysonType::PysonStrList: v.value = PysonValue::from_pyson_list(current_token); break;
    }

    return true;
    
}