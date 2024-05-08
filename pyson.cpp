#include "pyson.hpp"
#include <iterator>
#include <cstring>
#include <sstream>
#include <new>

#if POSIX_FUNCTIONS_AVAILABLE
#include <stdio.h>
#else
#include <fstream>
#endif

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

// PysonValue equality operator
bool PysonValue::operator== (const PysonValue& other) const noexcept {
    if (m_type != other.m_type)
        return false;
    
    switch (m_type) {
        case PysonType::PysonInt: return m_value.m_int == other.m_value.m_int;
        case PysonType::PysonFloat: return m_value.m_float == other.m_value.m_float;
        case PysonType::PysonStr: return m_value.m_str == other.m_value.m_str;
        case PysonType::PysonList: return m_value.m_list == other.m_value.m_list;
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
    switch (type()) {
        case PysonType::PysonInt: return "int";
        case PysonType::PysonFloat: return "float";
        case PysonType::PysonStr: return "str";
        case PysonType::PysonList: return "list";
    }
}

// Turn the PysonValue's value into a pyson-formatted string
std::string PysonValue::value_as_string() const noexcept {
    switch (type()) {
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

void PysonValue::force_to_string() noexcept {
    switch(type()) {
        case PysonType::PysonStr: return;
        default: *this = PysonValue(value_as_string());
    }
}

void PysonValue::force_to_list() noexcept {
    switch(type()) {
        case PysonType::PysonList: return;
        case PysonType::PysonStr:
            *this = PysonValue::from_pyson_list(m_value.m_str);
            return;
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            *this = PysonValue(std::vector<std::string>{value_as_string()});
    }
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

#if POSIX_FUNCTIONS_AVAILABLE
PysonFileReader::PysonFileReader(const char *path) : m_handle(fopen(path, "r")) {
    if (errno != 0) {
        throw std::runtime_error(
            "fopen() IO error code "
            + std::to_string(errno)
            + " in PysonFileReader::PysonFileReader(const char *path)"
        );
    }
}
PysonFileReader::PysonFileReader(const std::string& path) : m_handle(fopen(path.c_str(), "r")) {
    if (errno != 0) {
        throw std::runtime_error(
            "fopen() IO error code "
            + std::to_string(errno)
            + " in PysonFileReader::PysonFileReader(const std::string& path)"
        );
    }
}

std::optional<NamedPysonValue> PysonFileReader::next() {
    char *line = nullptr;
    ssize_t len = getline(&line, nullptr, m_handle);
    if (len == -1) return std::nullopt;
    
    
    std::istringstream str(std::string(line, len));
    NamedPysonValue result("", PysonValue(0));
    
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::next()");
}
NamedPysonValue PysonFileReader::next_or(const NamedPysonValue& default_val) {
    char *line = nullptr;
    ssize_t len = getline(&line, nullptr, m_handle);
    if (len == -1) return default_val;

    std::istringstream str(std::string(line, len));
    NamedPysonValue result("", PysonValue(0));
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::next_or()");
}
NamedPysonValue PysonFileReader::next_or(NamedPysonValue&& default_val) {
    char *line = nullptr;
    ssize_t len = getline(&line, nullptr, m_handle);
    if (len == -1) return std::move(default_val);

    std::istringstream str(std::string(line, len));
    NamedPysonValue result("", PysonValue(0));
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::next_or()");
}
NamedPysonValue PysonFileReader::next_or_throw() {
    char *line = nullptr;
    ssize_t len = getline(&line, nullptr, m_handle);
    if (len == -1)
        throw std::runtime_error("EOF encountered in PysonFileReader::next_or()");

    std::istringstream str(std::string(line, len));
    NamedPysonValue result("", PysonValue(0));
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::next_or_throw()");
}

void PysonFileReader::go_to_beginning() { rewind(m_handle); }
void PysonFileReader::go_to_line(size_t line_number) {
    go_to_beginning();
    char *line = nullptr;
    size_t len = 0;
    for (size_t i = 0; i < line_number; i++) {
        if (-1 != getline(&line, &len, m_handle)) continue;
        throw std::runtime_error("File ended before requested line in PysonFileReader::go_to_line()");
    }
}
void PysonFileReader::skip_n_lines(size_t amount_to_skip) {
    char *line = nullptr;
    size_t len = 0;
    for (size_t i = 0; i < amount_to_skip; i++) {
        if (-1 != getline(&line, &len, m_handle)) continue;
        throw std::runtime_error("File ended before requested line in PysonFileReader::skip_n_lines()");
    }
}

std::vector<NamedPysonValue> PysonFileReader::all() {
    go_to_beginning();
    char *line = nullptr;
    size_t len = 0;
    std::vector<NamedPysonValue> values{};
    NamedPysonValue next("", PysonValue(0));
    
    while (-1 != getline(&line, &len, m_handle)) {
        std::istringstream str(std::string(line, len));
        if (str >> next) values.push_back(next);
        else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::all()");
    }

    return values;
}

#else // windows
PysonFileReader::PysonFileReader(const char *path) : m_stream(std::ifstream(path)) {
    m_stream.open();
    if (!m_stream.good()) {
        throw std::runtime_error(
            "Error opening file in PysonFileReader::PysonFileReader(const char *path)"
        );
    }
}
PysonFileReader::PysonFileReader(const std::string& path) : m_stream(std::ifstream(path.c_str())) {
    m_stream.open();
    if (!m_stream.good()) {
        throw std::runtime_error(
            "Error opening file in PysonFileReader::PysonFileReader(const std::string& path)"
        );
    }
}

std::optional<NamedPysonValue> PysonFileReader::next() {
    NamedPysonValue v("", PysonValue(0));
    if (m_stream >> v) return v;
    else return std::nullopt;
}
NamedPysonValue PysonFileReader::next_or(const NamedPysonValue& default_value) {
    NamedPysonValue v("", PysonValue(0));
    if (m_stream >> v) return v;
    else return default_value;
}
NamedPysonValue PysonFileReader::next_or(NamedPysonValue&& default_value) {
    NamedPysonValue v("", PysonValue(0));
    if (m_stream >> v) return v;
    else return default_value;
}
NamedPysonValue PysonFileReader::next_or_throw() {
    NamedPysonValue v("", PysonValue(0));
    if (m_stream >> v) return v;
    else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::next_or_throw");
}

void PysonFileReader::go_to_beginning() {
    m_stream.clear();
    m_stream.seekg(0);
}
void PysonFileReader::go_to_line(size_t line_number) {
    go_to_beginning();
    for (size_t i = 0; i < line_number; i++) {
        if (m_stream.eof())
            throw std::runtime_error("File ended before requested line in PysonFileReader::go_to_line()");
        std::getline(m_stream);
    }
}
void PysonFileReader::skip_n_lines(size_t amount_to_skip) {
    for (size_t i = 0; i < amount_to_skip; i++) {
        if (m_stream.eof())
            throw std::runtime_error("File ended before skipping enough lines in PysonFileReader::skip_n_lines");
        std::getline(m_stream);
    }
}

std::vector<NamedPysonValue> PysonFileReader::all() {
    go_to_beginning();
    std::vector<NamedPysonValue> values{};
    NamedPysonValue next("", PysonValue(0));
    while (m_stream >> next)
        values.push_back(next);

    if (m_stream.eof()) return values;
    else throw std::runtime_error("Invalid pyson value encountered in PysonFileReader::all()");
}
#endif // functions that work for both
std::unordered_map<std::string, PysonValue> PysonFileReader::as_hashmap() {
    go_to_beginning();
    std::unordered_map<std::string, PysonValue> map;
    for (std::optional<NamedPysonValue> current = next(); current.has_value(); current = next()) {
        NamedPysonValue& cref = current.value();
        if (map.contains(cref.name()))
            throw std::runtime_error("Duplicate name encountered in PysonFileReader::as_hashmap()");
        map.insert(std::make_pair(cref.name(), cref.value()));
    }
    return map;
}

std::optional<PysonValue> PysonFileReader::value_with_name(const char *name) {
    go_to_beginning();
    for (std::optional<NamedPysonValue> current = next(); current.has_value(); current = next()) {
        if (current.value().name() == name)
            return current.value().value();
    }
    return std::nullopt;
}