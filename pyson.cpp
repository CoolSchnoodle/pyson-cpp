#include "pyson.hpp"
#include <iterator>
#include <cstring>
#include <sstream>
#include <new>

const char *WrongPysonType::what() const noexcept {
    switch (m_got) {
        case PysonType::PysonInt: switch (m_expected) {
            case PysonType::PysonInt: return nullptr; // unreachable
            case PysonType::PysonFloat: return "Wrong pyson type: expected int but got float";
            case PysonType::PysonStr: return "Wrong pyson type: expected int but got str";
            case PysonType::PysonList: return "Wrong pyson type: expected int but got list";
        }
        case PysonType::PysonFloat: switch (m_expected) {
            case PysonType::PysonFloat: return nullptr; // unreachable
            case PysonType::PysonInt: return "Wrong pyson type: expected float but got int";
            case PysonType::PysonStr: return "Wrong pyson type: expected float but got str";
            case PysonType::PysonList: return "Wrong pyson type: expected float but got list";
        }
        case PysonType::PysonStr: switch (m_expected) {
            case PysonType::PysonStr: return nullptr; // unreachable
            case PysonType::PysonInt: return "Wrong pyson type: expected str but got int";
            case PysonType::PysonFloat: return "Wrong pyson type: expected str but got float";
            case PysonType::PysonList: return "Wrong pyson type: expected str but got list";
        }
        case PysonType::PysonList: switch (m_expected) {
            case PysonType::PysonList: return nullptr; // unreachable
            case PysonType::PysonInt: return "Wrong pyson type: expected list but got int";
            case PysonType::PysonFloat: return "Wrong pyson type: expected list but got float";
            case PysonType::PysonStr: return "Wrong pyson type: expected list but got str";
        }
    }
}

// PysonValue destructor
PysonValue::~PysonValue() noexcept {
    switch (type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            return;
        
        case PysonType::PysonStr:
            m_value.m_str.~basic_string();
            return;
        
        case PysonType::PysonList:
            m_value.m_list.~vector();
            return;
    }
}

// makes a PysonValue printable
std::ostream& operator<< (std::ostream& o, const PysonValue& val) {
    o << val.type_cstring() << ':' << val.value_as_string();
    return o;
}

// PysonValue copy constructor
PysonValue::PysonValue(const PysonValue& other) : m_value(0) {
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            return;

        case PysonType::PysonStr:
            new(&m_value.m_str) std::string(other.m_value.m_str);
            m_type = PysonType::PysonStr;
            return;

        case PysonType::PysonList:
            new(&m_value.m_list) std::vector<std::string>(other.m_value.m_list);
            m_type = PysonType::PysonList;
            return;
    }
}
// PysonValue move constructor
PysonValue::PysonValue(PysonValue&& other) : m_value(0) {
    // (same code as copy constructor)
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            new(&m_value.m_str) std::string(std::move(other.m_value.m_str));
            m_type = PysonType::PysonStr;
            break;

        case PysonType::PysonList:
            new(&m_value.m_list) std::vector<std::string>(std::move(other.m_value.m_list));
            m_type = PysonType::PysonList;
            break;
    }
}
// PysonValue copy assignment
PysonValue& PysonValue::operator= (const PysonValue& other) {
    this->~PysonValue();
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            new(&m_value.m_str) std::string(other.m_value.m_str);
            m_type = PysonType::PysonStr;
            break;

        case PysonType::PysonList:
            new(&m_value.m_list) std::vector<std::string>(other.m_value.m_list);
            m_type = PysonType::PysonList;
            break;
    }
    return *this;
}
// PysonValue move assignment
PysonValue& PysonValue::operator= (PysonValue&& other) {
    this->~PysonValue();
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(PysonValue));
            break;

        case PysonType::PysonStr:
            m_type = PysonType::PysonStr;
            new(&m_value.m_str) std::string(std::move(other.m_value.m_str));
            break;

        case PysonType::PysonList:
            m_type = PysonType::PysonList;
            new(&m_value.m_list) std::vector<std::string>(std::move(other.m_value.m_list));
            break;
    }
    return *this;
}

// Returns "int", "float", "str", or "list"
const char *PysonValue::type_cstring() const noexcept {
    switch (this->type()) {
        case PysonType::PysonInt: return "int";
        case PysonType::PysonFloat: return "float";
        case PysonType::PysonStr: return "str";
        case PysonType::PysonList: return "list";
    }
}

// Turn the PysonValue's value into a pyson-formatted string
std::string PysonValue::value_as_string() const noexcept {
    switch (this->type()) {
        case PysonType::PysonInt: return std::to_string(m_value.m_int);
        case PysonType::PysonFloat: return std::to_string(m_value.m_float);
        case PysonType::PysonStr: return m_value.m_str;
        case PysonType::PysonList: {
            std::ostringstream pyson_list;
            if (m_value.m_list.empty()) return "";
            std::copy(
                m_value.m_list.begin(),
                m_value.m_list.end(),
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
    PysonValue result(std::vector<std::string>{});
    std::string current_token{};
    for (char c : pyson_list) {
        current_token.push_back(c);
        size_t size = current_token.length();
        if (size > 3 && current_token.substr(size-3, 3) == "(*)") {
            result.m_value.m_list.push_back(current_token.substr(0, size-3));
            current_token.clear();
        }
    }
    result.m_value.m_list.push_back(current_token);
    return result;
}

// Output a NamedPysonValue in the pyson format
std::ostream& operator<< (std::ostream& o, NamedPysonValue& v) {
    o << v.m_name << ':' << v.m_value;
    return o;
}

// Read a pyson-formatted line into a NamedPysonValue
bool operator>> (std::istream& i, NamedPysonValue& v) {
    std::string current_token{};
    std::getline(i, current_token, ':');
    for (char c : current_token) if (c == '\n') return false;
    v.change_name(current_token);

    std::getline(i, current_token, ':');
    if (current_token == "int") { v.m_value.m_type = PysonType::PysonInt; }
    else if (current_token == "float") { v.m_value.m_type = PysonType::PysonFloat; }
    else if (current_token == "str") { v.m_value = PysonValue(""); }
    else if (current_token == "list") { v.m_value = PysonValue(std::vector<std::string>{}); }
    else { return false; }

    std::getline(i, current_token);
    switch (v.m_value.type()) {
        case PysonType::PysonInt:
            try { v.m_value = PysonValue(std::stoi(current_token)); break; }
            catch (...) { return false; }
        case PysonType::PysonFloat:
            try { v.m_value = PysonValue(std::stod(current_token)); break; }
            catch(...) { return false; }
        case PysonType::PysonStr: v.m_value = PysonValue(current_token); break;
        case PysonType::PysonList: v.m_value = PysonValue::from_pyson_list(current_token); break;
    }
    return true;
}