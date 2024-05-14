#include "pyson.hpp"
#include <iterator>
#include <cstring>
#include <sstream>
#include <new>
#include <limits>
#include <iostream>

namespace pyson {

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

// Value destructor
Value::~Value() noexcept {
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

// Value equality operator
bool Value::operator== (const Value& other) const noexcept {
    if (m_type != other.m_type)
        return false;
    
    switch (m_type) {
        case PysonType::PysonInt: return m_value.m_int == other.m_value.m_int;
        case PysonType::PysonFloat: return m_value.m_float == other.m_value.m_float;
        case PysonType::PysonStr: return m_value.m_str == other.m_value.m_str;
        case PysonType::PysonList: return m_value.m_list == other.m_value.m_list;
    }
}

// makes a Value printable
std::ostream& operator<< (std::ostream& o, const Value& val) {
    o << val.type_cstring() << ':' << val.value_as_string();
    return o;
}

// Value copy constructor
Value::Value(const Value& other) : m_value(0) {
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(Value));
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
// Value move constructor
Value::Value(Value&& other) : m_value(0) {
    // (same code as copy constructor)
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(Value));
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
// Value copy assignment
Value& Value::operator= (const Value& other) {
    this->~Value();
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(Value));
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
// Value move assignment
Value& Value::operator= (Value&& other) {
    this->~Value();
    switch (other.type()) {
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            std::memcpy(this, &other, sizeof(Value));
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
const char *Value::type_cstring() const noexcept {
    switch (type()) {
        case PysonType::PysonInt: return "int";
        case PysonType::PysonFloat: return "float";
        case PysonType::PysonStr: return "str";
        case PysonType::PysonList: return "list";
    }
}

// Turn the Value's value into a pyson-formatted string
std::string Value::value_as_string() const noexcept {
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

// Create a Value from a list in the pyson format
Value Value::from_pyson_list(std::string pyson_list) {
    Value result(std::vector<std::string>{});
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

void Value::force_to_string() noexcept {
    switch(type()) {
        case PysonType::PysonStr: return;
        default: *this = Value(value_as_string());
    }
}

void Value::force_to_list() noexcept {
    switch(type()) {
        case PysonType::PysonList: return;
        case PysonType::PysonStr:
            *this = Value::from_pyson_list(m_value.m_str);
            return;
        case PysonType::PysonInt:
        case PysonType::PysonFloat:
            *this = Value(std::vector<std::string>{value_as_string()});
    }
}

// Output a NamedValue in the pyson format
std::ostream& operator<< (std::ostream& o, NamedValue& v) {
    o << v.m_name << ':' << v.m_value;
    return o;
}

// Read a pyson-formatted line into a NamedValue
bool operator>> (std::istream& i, NamedValue& v) {
    std::string current_token{};
    std::getline(i, current_token, ':');
    for (char c : current_token) if (c == '\n') return false;
    v.change_name(current_token);

    std::getline(i, current_token, ':');
    if (current_token == "int") { v.m_value.m_type = PysonType::PysonInt; }
    else if (current_token == "float") { v.m_value.m_type = PysonType::PysonFloat; }
    else if (current_token == "str") { v.m_value = Value(""); }
    else if (current_token == "list") { v.m_value = Value(std::vector<std::string>{}); }
    else { return false; }

    std::getline(i, current_token);
    switch (v.m_value.type()) {
        case PysonType::PysonInt:
            try { v.m_value = Value(std::stoi(current_token)); break; }
            catch (...) { return false; }
        case PysonType::PysonFloat:
            try { v.m_value = Value(std::stod(current_token)); break; }
            catch(...) { return false; }
        case PysonType::PysonStr: v.m_value = Value(current_token); break;
        case PysonType::PysonList: v.m_value = Value::from_pyson_list(current_token); break;
    }
    return true;
}

#if POSIX_FUNCTIONS_AVAILABLE
FileReader::FileReader(const char *path) : m_handle(fopen(path, "r")) {
    if (errno != 0) {
        throw std::runtime_error(
            "fopen() IO error code "
            + std::to_string(errno)
            + " in FileReader::FileReader(const char *path)"
        );
    }
}
FileReader::FileReader(const std::string& path) : m_handle(fopen(path.c_str(), "r")) {
    if (errno != 0) {
        throw std::runtime_error(
            "fopen() IO error code "
            + std::to_string(errno)
            + " in FileReader::FileReader(const std::string& path)"
        );
    }
}

std::optional<NamedValue> FileReader::next() {
    char *line = nullptr;
    size_t s{};
    ssize_t len = getline(&line, &s, m_handle);
    if (len == -1) return std::nullopt;
    
    
    std::istringstream str(std::string(line, len));
    free((void*)line);
    NamedValue result("", Value(0));
    
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in FileReader::next()");
}
NamedValue FileReader::next_or(const NamedValue& default_val) {
    char *line = nullptr;
    size_t s{};
    ssize_t len = getline(&line, &s, m_handle);
    if (len == -1) return default_val;

    std::istringstream str(std::string(line, len));
    free((void*)line);
    NamedValue result("", Value(0));
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in FileReader::next_or()");
}
NamedValue FileReader::next_or(NamedValue&& default_val) {
    char *line = nullptr;
    size_t s{};
    ssize_t len = getline(&line, &s, m_handle);
    if (len == -1) return std::move(default_val);

    std::istringstream str(std::string(line, len));
    NamedValue result("", Value(0));
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in FileReader::next_or()");
}
NamedValue FileReader::next_or_throw() {
    char *line = nullptr;
    size_t s{};
    ssize_t len = getline(&line, &s, m_handle);
    if (len == -1)
        throw std::runtime_error("EOF encountered in FileReader::next_or()");

    std::istringstream str(std::string(line, len));
    free((void*)line);
    NamedValue result("", Value(0));
    if (str >> result) return result;
    else throw std::runtime_error("Invalid pyson value encountered in FileReader::next_or_throw()");
}

void FileReader::go_to_beginning() { rewind(m_handle); }
void FileReader::go_to_line(size_t line_number) {
    go_to_beginning();
    if (line_number == 0) return;
    
    char *line = nullptr;
    size_t len = 0;
    for (size_t i = 0; i < line_number; i++) {
        if (-1 != getline(&line, &len, m_handle)) continue;
        free((void*)line);
        throw std::runtime_error("File ended before requested line in FileReader::go_to_line()");
    }
    free((void*)line);
}
void FileReader::skip_n_lines(size_t amount_to_skip) {
    if (amount_to_skip == 0) return;
    
    char *line = nullptr;
    size_t len = 0;
    for (size_t i = 0; i < amount_to_skip; i++) {
        if (-1 != getline(&line, &len, m_handle)) continue;
        free((void*)line);
        throw std::runtime_error("File ended before requested line in FileReader::skip_n_lines()");
    }
    free((void*)line);
}

std::vector<NamedValue> FileReader::all() {
    go_to_beginning();
    char *line = nullptr;
    size_t len = 0;
    std::vector<NamedValue> values{};
    NamedValue next("", Value(0));
    
    while (-1 != getline(&line, &len, m_handle)) {
        std::istringstream str(std::string(line, len));
        if (str >> next) values.push_back(next);
        else {
            free((void*)line);
            throw std::runtime_error("Invalid pyson value encountered in FileReader::all()");
        }
    }

    free((void*)line);
    return values;
}

#else // windows
FileReader::FileReader(const char *path) : m_stream() {
    m_stream.open(path);
    if (!m_stream.good()) {
        throw std::runtime_error(
            "Error opening file in FileReader::FileReader(const char *path)"
        );
    }
}
FileReader::FileReader(const std::string& path) : m_stream() {
    m_stream.open(path.c_str());
    if (!m_stream.good()) {
        throw std::runtime_error(
            "Error opening file in FileReader::FileReader(const std::string& path)"
        );
    }
}

std::optional<NamedValue> FileReader::next() {
    NamedValue v("", Value(0));
    if (m_stream >> v) return v;
    else return std::nullopt;
}
NamedValue FileReader::next_or(const NamedValue& default_value) {
    NamedValue v("", Value(0));
    if (m_stream >> v) return v;
    else return default_value;
}
NamedValue FileReader::next_or(NamedValue&& default_value) {
    NamedValue v("", Value(0));
    if (m_stream >> v) return v;
    else return std::move(default_value);
}
NamedValue FileReader::next_or_throw() {
    NamedValue v("", Value(0));
    if (m_stream >> v) return v;
    else throw std::runtime_error("Invalid pyson value encountered in FileReader::next_or_throw");
}

void FileReader::go_to_beginning() {
    m_stream.clear();
    m_stream.seekg(0);
}
void FileReader::go_to_line(size_t line_number) {
    go_to_beginning();
    for (size_t i = 0; i < line_number; i++) {
        if (m_stream.eof())
            throw std::runtime_error("File ended before requested line in FileReader::go_to_line()");
        m_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}
void FileReader::skip_n_lines(size_t amount_to_skip) {
    for (size_t i = 0; i < amount_to_skip; i++) {
        if (m_stream.eof())
            throw std::runtime_error("File ended before skipping enough lines in FileReader::skip_n_lines");
        m_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::vector<NamedValue> FileReader::all() {
    go_to_beginning();
    std::vector<NamedValue> values{};
    NamedValue next("", Value(0));
    while (m_stream >> next)
        values.push_back(next);

    if (m_stream.eof()) return values;
    else throw std::runtime_error("Invalid pyson value encountered in FileReader::all()");
}
#endif // functions that work for both
std::unordered_map<std::string, Value> FileReader::as_hashmap() {
    go_to_beginning();
    std::unordered_map<std::string, Value> map;
    for (std::optional<NamedValue> current = next(); current.has_value(); current = next()) {
        NamedValue& cref = current.value();
        if (map.contains(cref.name()))
            throw std::runtime_error("Duplicate name encountered in FileReader::as_hashmap()");
        map.insert(std::make_pair(cref.name(), cref.value()));
    }
    return map;
}

std::optional<Value> FileReader::value_with_name(const char *name) {
    go_to_beginning();
    for (std::optional<NamedValue> current = next(); current.has_value(); current = next()) {
        if (current.value().name() == name)
            return current.value().value();
    }
    return std::nullopt;
}

void FileReader::for_each(std::function<void (NamedValue)> predicate) {
    for (auto v = next(); v != std::nullopt; v = next())
        predicate(v.value());
}

template <class Return>
std::vector<Return> FileReader::map_each(std::function<Return (NamedValue)> predicate) {
    auto vec = std::vector<Return>{};
    for (auto v = next(); v != std::nullopt; v = next())
        vec.push_back(predicate(v));
    return vec;
}

void FileReader::for_each_while(std::function<bool (NamedValue)> predicate) {
    for (auto v = next(); v != std::nullopt && predicate(v.value()); v = next())
        ; // no loop body
}

template <class Return>
std::vector<Return> FileReader::map_while(std::function<std::pair<bool, Return> (NamedValue)> predicate) {
    auto vec = std::vector<Return>{};
    for (auto val = next(); val != std::nullopt; val = next()) {
        auto res = predicate(val);
        if (!res.first) return vec;
        else vec.push_back(res.second);
    }
    return vec;
}

template <class Return>
std::vector<Return> FileReader::map_while(std::function<std::optional<Return> (NamedValue)> predicate) {
    auto vec = std::vector<Return>{};
    for (auto val = next(); val != std::nullopt; val = next()) {
        auto res = predicate(val);
        if (!res.has_value()) return vec;
        else vec.push_back(res.value());
    }
    return vec;
}

FileReader::Iter::Iter(FileReader *reader) : m_reader(reader), m_cached("", Value(0)) {
    if (m_reader == nullptr)
        return;
    auto opt = m_reader->next();
    if (opt == std::nullopt) {
        m_reader = nullptr;
        return;
    }
    m_cached = opt.value();
}

FileReader::End FileReader::end() { return End{}; }
FileReader::Iter FileReader::begin() {
#if POSIX_FUNCTIONS_AVAILABLE
    int c;
    if ((c = fgetc(m_handle)) == EOF)
        return Iter(nullptr);
    ungetc(c, m_handle);
    return Iter(this);
#else
    if (m_stream.peek() == EOF)
        return Iter(nullptr);
    else return Iter(this);
#endif
}

void FileReader::Iter::operator++() {
    if (m_reader == nullptr)
        return;
    auto opt = m_reader->next();
    if (!opt.has_value()) {
        m_reader = nullptr;
        return;
    }
    m_cached = opt.value();
}

NamedValue FileReader::Iter::operator*() {
    if (m_reader == nullptr)
        throw std::logic_error("Tried to dereference finished FileReader::Iter iterator");
    
    return m_cached;
}

bool FileReader::Iter::operator!=(const End& end) {
    (void)end;
    return m_reader != nullptr;
}

}