#ifndef PYSON_HPP_PYSON_INCLUDED
#define PYSON_HPP_PYSON_INCLUDED

#include <string>
#include <vector>
#include <iosfwd>
#include <optional>
#include <cstring>
#include <stdexcept>
#include <array>
#include <unordered_map>

class PysonValue;
class NamedPysonValue;
class PysonFileReader;

/**
 * An enum that says which type a PysonValue is.
 * Values in pyson can either be integers, 64-bit floating point numbers, strings, or lists of strings.
 */
enum class PysonType {
    PysonInt,
    PysonFloat,
    PysonStr,
    PysonList,
};

/**
 * An exception for when a specific pyson type was expected, but a different type was recieved.
 * WrongPysonType inherits from std::exception, so you can catch it as a std::exception&
 * just like most exceptions.
 */
class WrongPysonType : public std::exception {
    PysonType m_expected;
    PysonType m_got;
public:
    WrongPysonType(PysonType expected, PysonType got) : m_expected(expected), m_got(got) {
        if (expected == got)
            throw std::logic_error("You can't have the wrong pyson type if you got what you expected");
    }
    const char *what() const noexcept override;
};

/**
 * The union within a PysonValue that contains either an integer, 64-bit float, string, or list of strings.
 * As a user of pyson, you will never need to know about or use this.
 */
union PysonValueInner {
    friend class PysonValue;
        friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);
    friend class NamedPysonValue;

private:
    int m_int;
    double m_float;
    std::string m_str;
    std::vector<std::string> m_list;

    /// Users of pyson can't get a PysonValueInner, so the PysonValueInner constructors are private.
    /// Only a PysonValue can get one and their destructor takes care of destructing the correct way.
    PysonValueInner(int val) noexcept : m_int(val) {}
    PysonValueInner(double val) noexcept : m_float(val) {}
    PysonValueInner(const std::string& str) noexcept : m_str(str) {}
    PysonValueInner(std::string&& str) noexcept : m_str(str) {}
    PysonValueInner(const std::vector<std::string>& list) noexcept : m_list(list) {}
    PysonValueInner(std::vector<std::string>&& list) noexcept : m_list(list) {}

    ~PysonValueInner() noexcept {}
};

/**
 * A value from a pyson file.
 * Structurally, contains a PysonValueInner union and a PysonType tag.
 */
class PysonValue {

    template <typename T> using vector = std::vector<T>;
    template <typename T> using optional = std::optional<T>;
    using string = std::string;

    PysonType m_type;
    PysonValueInner m_value;

public:
    friend class PysonFileReader;
    friend class NamedPysonValue;
        friend bool operator>> (std::istream& i, NamedPysonValue& v);
    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    bool operator== (const PysonValue& other) const noexcept;

    /// Get the type of the PysonValue as a PysonType
    PysonType type() const noexcept { return m_type; }
    /// Get the type of the PysonValue as a C string (const char *)
    const char *type_cstring() const noexcept;
    /// Get the type of the PysonValue as a C++ string (std::string)
    string type_string() const noexcept { return this->type_cstring(); }

    /// Returns whether the PysonValue is an integer
    bool is_int() const noexcept { return this->type() == PysonType::PysonInt; }
    /// Returns whether the PysonValue is a floating-point number
    bool is_float() const noexcept { return this->type() == PysonType::PysonFloat; }
    /// Returns whether the PysonValue is a string
    bool is_str() const noexcept { return this->type() == PysonType::PysonStr; }
    /// Returns whether the PysonValue is a list of strings
    bool is_list() const noexcept { return this->type() == PysonType::PysonList; }

    // Returns the PysonValue's value as a string
    string value_as_string() const noexcept;

    /// Construct a PysonValue from another PysonValue
    PysonValue(const PysonValue&);
    PysonValue(PysonValue&&);
    /// Assign a PysonValue to the value of another PysonValue
    PysonValue& operator= (const PysonValue&);
    PysonValue& operator= (PysonValue&&);

    /// Construct a PysonValue from a string formatted as a pyson list
    static PysonValue from_pyson_list(string pyson_list);

    /// Construct a PysonValue from an integer
    explicit PysonValue(int val) : m_type(PysonType::PysonInt), m_value(val) {}
    /// Construct a PysonValue from a 64-bit floating-point number
    explicit PysonValue(double val) : m_type(PysonType::PysonFloat), m_value(val) {}
    /// Construct a PysonValue from a string
    explicit PysonValue(const string& str) : m_type(PysonType::PysonStr), m_value(str) {}
    explicit PysonValue(string&& str) : m_type(PysonType::PysonStr), m_value(str) {}
    /// Construct a PysonValue from a list of strings
    explicit PysonValue(const vector<string>& list) : m_type(PysonType::PysonList), m_value(list) {}
    explicit PysonValue(vector<string>&& list) : m_type(PysonType::PysonList), m_value(list) {}

    /// Destruct a PysonValue, including correctly destructing the PysonValueInner
    ~PysonValue() noexcept;

    /**
     * Get the int from the PysonValue, or a custom default value.
     * All integers are valid in pyson, so the default value will also
     * be returned if that was the actual value.
     */
    int int_or(int default_val) const noexcept {
        switch (type()) {
            case PysonType::PysonInt: return m_value.m_int;
            default: return default_val;
        }
    }
    /**
     * Get the 64-bit float from the PysonValue, or a custom defualt value.
     * All 64-bit floats are valid in pyson, so the default value will also
     * be returned if that was the actual value.
     */
    double float_or(double default_val) const noexcept {
        switch (type()) {
            case PysonType::PysonFloat: return m_value.m_float;
            default: return default_val;
        }
    }
    /**
     * Get the string from the PysonValue, or a custom default value.
     * All strings that don't contain newlines are valid in pyson,
     * so the default may be returned if it was the actual value
     */
    string string_or(string default_val) const noexcept {
        switch (type()) {
            case PysonType::PysonStr: return m_value.m_str;
            default: return default_val;
        }
    }
    /**
     * Get the list from the PysonValue, or a custom default value.
     * All strings without newlines are valid in pyson,
     * so the default may be returned if it was the actual value.
     */
    vector<string> list_or(vector<string> default_val) const noexcept {
        switch(type()) {
            case PysonType::PysonList: return m_value.m_list;
            default: return default_val;
        }
    }

    /**
     * Get the int from the PysonValue, or 0 if it was not an int
     * Note: if this returns 0 that does not necessarily mean that it was not an int,
     * 0 is a valid integer value in pyson
     */
    int int_or_zero() const noexcept { return int_or(0); }
    /**
     * Get the 64-bit float from the PysonValue, or 0.0 if it was not a float
     * Note: if this returns 0.0 that does not necessarily mean it was not a float,
     * 0.0 is a valid float value in pyson
     */
    double float_or_zero() const noexcept { return float_or(0.0); }
    /**
     * Get the string from the PysonValue, or an empty string if it was not a string
     * Note: if this returns an empty string that does not necessarily mean it was not a string,
     * empty strings are valid in pyson
     */
    string string_or_empty() const noexcept { return string_or(""); }
    /**
     * Get the list of strings from the PysonValue, or an empty list if the value was not a list.
     * Note: if this returns and empty list that does not necessarily mean it was not a list,
     * empty lists are valid in pyson.
     */
    vector<string> list_or_empty() const noexcept { return list_or(vector<string>{}); }

    /// Get the integer from the PysonValue, or a null option if the value isn't an integer
    optional<int> get_int() const noexcept {
        switch (type()) {
            case PysonType::PysonInt: return m_value.m_int;
            default: return std::nullopt;
        }
    }
    /// Get the 64-bit float from the PysonValue, or a null option if the value isn't a float
    optional<double> get_float() const noexcept {
        switch(type()) {
            case PysonType::PysonFloat: return m_value.m_float;
            default: return std::nullopt;
        }
    }
    /// Get the string from the PysonValue, or a null option if the value isn't a string
    optional<string> get_string() const noexcept {
        switch(type()) {
            case PysonType::PysonStr: return m_value.m_str;
            default: return std::nullopt;
        }
    }
    /// Get the list from the PysonValue, or a null option if the value isn't a list
    optional<vector<string>> get_list() const noexcept {
        switch(type()) {
            case PysonType::PysonList: return m_value.m_list;
            default: return std::nullopt;
        }
    }

    /// Get the int from the PysonValue, or throw a WrongPysonType if it isn't an int
    int int_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonInt;
        switch (found_type) {
            case expected_type: return m_value.m_int;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }
    /// Get the 64-bit float from the PysonValue, or throw a WrongPysonType if it isn't a float
    double float_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonFloat;
        switch (found_type) {
            case expected_type: return m_value.m_float;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }
    /// Get the string from the PysonValue, or throw a WrongPysonType if it isn't a string
    string string_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonStr;
        switch (found_type) {
            case expected_type: return m_value.m_str;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }
    /// Get the list from the PysonValue, or throw a WrongPysonType if it isn't a list
    vector<string> list_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonList;
        switch (found_type) {
            case expected_type: return m_value.m_list;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }

    /**
     * Make the value become a string, no matter what it was previously.
     * The string will be identical to the string returned by value_as_string().
     */
    void force_to_string() noexcept;
    /**
     * Make the value become a list of strings, no matter what it was previously.
     * If the value is an integer or floating-point number, the list will have 1 element
     * that is the string representation of that number.
     * If the value is a string, it will be parsed as a pyson list.
     * If the value is a list, it will remain unchanged.
     */
    void force_to_list() noexcept;
};

/// A PysonValue, but with a name
class NamedPysonValue {
    std::string m_name;
    PysonValue m_value;

public:
    friend class PysonFileReader;
    /// Print a NamedPysonValue in the pyson format
    friend std::ostream& operator<< (std::ostream& o, NamedPysonValue& v);
    /// Read in a NamedPysonValue using the pyson format
    friend bool operator>> (std::istream& i, NamedPysonValue& v);

    /// Construct a NamedPysonValue from a name and a PysonValue
    explicit NamedPysonValue(const std::string& name, const PysonValue& value) : m_name(name), m_value(value) {}
    explicit NamedPysonValue(const std::string& name, PysonValue&& value) : m_name(name), m_value(value) {}
    explicit NamedPysonValue(std::string&& name, const PysonValue& value) : m_name(name), m_value(value) {}
    explicit NamedPysonValue(std::string&& name, PysonValue&& value) : m_name(name), m_value(value) {}

    /// Returns the name of the NamedPysonValue
    std::string name() const noexcept { return m_name; }
    /// Returns the PysonValue contained by the NamedPysonValue
    PysonValue value() const noexcept { return m_value; }

    /// Change the name of a NamedPysonValue
    void change_name(const std::string& new_name) noexcept { m_name = new_name; }
    /// Swap out the value of a NamedPysonValue, keeping the name
    void change_value(const PysonValue& new_value) noexcept { m_value = new_value; }
};

class PysonFileReader {
    FILE *m_handle;

public:
    PysonFileReader(const char *path) : m_handle(fopen(path, "r")) {
        if (errno != 0) {
            throw std::runtime_error(
                "fopen() IO error code "
                + std::to_string(errno)
                + " in PysonFileReader::PysonFileReader(const char *path)"
            );
        }
    }
    PysonFileReader(std::string path) : m_handle(fopen(path.c_str(), "r")) {
        if (errno != 0) {
            throw std::runtime_error(
                "fopen() IO error code "
                + std::to_string(errno)
                + " in PysonFileReader::PysonFileReader(const char *path)"
            );
        }
    }

    /// Get the next NamedPysonValue from the file,
    /// or the null option if the file ended
    std::optional<NamedPysonValue> next();
    /// Get the next NamedPysonValue from the file,
    /// or a predetermined default NamedPysonValue if the file ended
    NamedPysonValue next_or(const NamedPysonValue& default_value);
    NamedPysonValue next_or(NamedPysonValue&& default_value);
    /// Get the next NamedPysonValue from the file,
    /// or throw an exception if the file ended
    NamedPysonValue next_or_throw();

    /**
     * Get a vector that contains each NamedPysonValue from the file.
     * This call will read the entire file,
     * not just the portion after the current read position.
     */
    std::vector<NamedPysonValue> all();

    /// Get a hashmap of each name to its PysonValue from the file.
    /// This call will read the entire file,
    /// not just the portion after the current read position.
    std::unordered_map<std::string, PysonValue> as_hashmap();

    /// Reset read progress to the beginning of the file
    void go_to_beginning();

    /// Go to a specific line in the file, start reading from that line
    void go_to_line(size_t line);

    /// Skip the next N lines
    void skip_n_lines(size_t amount_to_skip);

    /**
     * Locate the PysonValue with a specific name from the file.
     * The value will be found if it exists anywhere in the file,
     * even before the current read location.
     * The next PysonValue read by next() will be the one after the value found.
     */
    std::optional<PysonValue> value_with_name(const char *name);
    std::optional<PysonValue> value_with_name(std::string name) { return value_with_name(name.c_str()); }
};

#endif