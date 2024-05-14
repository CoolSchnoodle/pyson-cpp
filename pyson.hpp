#ifndef PYSON_HPP_PYSON_INCLUDED
#define PYSON_HPP_PYSON_INCLUDED

// OS check
#if defined(__unix__)
#define POSIX_FUNCTIONS_AVAILABLE 1
#elif defined(__APPLE__)
#define POSIX_FUNCTIONS_AVAILABLE 1
#else
#define POSIX_FUNCTIONS_AVAILABLE 0
#endif

#include <string>
#include <vector>
#include <iosfwd>
#include <optional>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <functional>

#if POSIX_FUNCTIONS_AVAILABLE
#include <stdio.h>
#else
#include <fstream>
#endif

namespace pyson {

class Value;
class NamedValue;
class FileReader;

/**
 * An enum that says which type a Value is.
 * Values in pyson can either be integers, 64-bit floating point numbers, strings, or lists of strings.
 */
enum class PysonType : unsigned char {
    PysonInt = 0,
    PysonFloat = 1,
    PysonStr = 2,
    PysonList = 4,
};

/**
 * An exception for when a specific pyson type was expected, but a different type was recieved.
 * WrongPysonType inherits from std::exception, so you can catch it as a std::exception&
 * just like most exceptions. Warning: the constructor of WrongPysonType will
 * throw an exception if the type you expected is the same type as you got,
 * this may cause an unrecoverable crash if it happens during exception handling.
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
 * The union within a Value that contains either an integer, 64-bit float, string, or list of strings.
 * As a user of pyson, you will never need to know about or use this.
 */
union ValueInner {
    friend class Value;
        friend std::ostream& operator<< (std::ostream& o, const Value& val);
    friend class NamedValue;

private:
    int m_int;
    double m_float;
    std::string m_str;
    std::vector<std::string> m_list;

    /// Users of pyson can't get a ValueInner, so the ValueInner constructors are private.
    /// Only a Value can get one and their destructor takes care of destructing the correct way.
    ValueInner(int val) noexcept : m_int(val) {}
    ValueInner(double val) noexcept : m_float(val) {}
    ValueInner(const std::string& str) noexcept : m_str(str) {}
    ValueInner(std::string&& str) noexcept : m_str(str) {}
    ValueInner(const std::vector<std::string>& list) noexcept : m_list(list) {}
    ValueInner(std::vector<std::string>&& list) noexcept : m_list(list) {}

    ~ValueInner() noexcept {}
};

/**
 * A value from a pyson file.
 * Structurally, contains a ValueInner union and a PysonType tag.
 */
class Value {

    template <typename T> using vector = std::vector<T>;
    template <typename T> using optional = std::optional<T>;
    using string = std::string;

    PysonType m_type;
    ValueInner m_value;

public:
    friend class FileReader;
    friend class NamedValue;
        friend bool operator>> (std::istream& i, NamedValue& v);
    friend std::ostream& operator<< (std::ostream& o, const Value& val);

    bool operator== (const Value& other) const noexcept;

    /// Get the type of the Value as a PysonType
    PysonType type() const noexcept { return m_type; }
    /// Get the type of the Value as a C string (const char *)
    const char *type_cstring() const noexcept;
    /// Get the type of the Value as a C++ string (std::string)
    string type_string() const noexcept { return this->type_cstring(); }

    /// Returns whether the Value is an integer
    bool is_int() const noexcept { return this->type() == PysonType::PysonInt; }
    /// Returns whether the Value is a floating-point number
    bool is_float() const noexcept { return this->type() == PysonType::PysonFloat; }
    /// Returns whether the Value is a string
    bool is_str() const noexcept { return this->type() == PysonType::PysonStr; }
    /// Returns whether the Value is a list of strings
    bool is_list() const noexcept { return this->type() == PysonType::PysonList; }

    // Returns the Value's value as a string
    string value_as_string() const noexcept;

    /// Construct a Value from another Value
    Value(const Value&);
    Value(Value&&);
    /// Assign a Value to the value of another Value
    Value& operator= (const Value&);
    Value& operator= (Value&&);

    /// Construct a Value from a string formatted as a pyson list
    static Value from_pyson_list(string pyson_list);

    /// Construct a Value from an integer
    explicit Value(int val) : m_type(PysonType::PysonInt), m_value(val) {}
    /// Construct a Value from a 64-bit floating-point number
    explicit Value(double val) : m_type(PysonType::PysonFloat), m_value(val) {}
    /// Construct a Value from a string
    explicit Value(const string& str) : m_type(PysonType::PysonStr), m_value(str) {}
    explicit Value(string&& str) : m_type(PysonType::PysonStr), m_value(str) {}
    /// Construct a Value from a list of strings
    explicit Value(const vector<string>& list) : m_type(PysonType::PysonList), m_value(list) {}
    explicit Value(vector<string>&& list) : m_type(PysonType::PysonList), m_value(list) {}

    /// Destruct a Value, including correctly destructing the ValueInner
    ~Value() noexcept;

    /**
     * Get the int from the Value, or a custom default value.
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
     * Get the 64-bit float from the Value, or a custom defualt value.
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
     * Get the string from the Value, or a custom default value.
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
     * Get the list from the Value, or a custom default value.
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
     * Get the int from the Value, or 0 if it was not an int
     * Note: if this returns 0 that does not necessarily mean that it was not an int,
     * 0 is a valid integer value in pyson
     */
    int int_or_zero() const noexcept { return int_or(0); }
    /**
     * Get the 64-bit float from the Value, or 0.0 if it was not a float
     * Note: if this returns 0.0 that does not necessarily mean it was not a float,
     * 0.0 is a valid float value in pyson
     */
    double float_or_zero() const noexcept { return float_or(0.0); }
    /**
     * Get the string from the Value, or an empty string if it was not a string
     * Note: if this returns an empty string that does not necessarily mean it was not a string,
     * empty strings are valid in pyson
     */
    string string_or_empty() const noexcept { return string_or(""); }
    /**
     * Get the list of strings from the Value, or an empty list if the value was not a list.
     * Note: if this returns and empty list that does not necessarily mean it was not a list,
     * empty lists are valid in pyson.
     */
    vector<string> list_or_empty() const noexcept { return list_or(vector<string>{}); }

    /// Get the integer from the Value, or a null option if the value isn't an integer
    optional<int> get_int() const noexcept {
        switch (type()) {
            case PysonType::PysonInt: return m_value.m_int;
            default: return std::nullopt;
        }
    }
    /// Get the 64-bit float from the Value, or a null option if the value isn't a float
    optional<double> get_float() const noexcept {
        switch(type()) {
            case PysonType::PysonFloat: return m_value.m_float;
            default: return std::nullopt;
        }
    }
    /// Get the string from the Value, or a null option if the value isn't a string
    optional<string> get_string() const noexcept {
        switch(type()) {
            case PysonType::PysonStr: return m_value.m_str;
            default: return std::nullopt;
        }
    }
    /// Get the list from the Value, or a null option if the value isn't a list
    optional<vector<string>> get_list() const noexcept {
        switch(type()) {
            case PysonType::PysonList: return m_value.m_list;
            default: return std::nullopt;
        }
    }

    /// Get the int from the Value, or throw a WrongPysonType if it isn't an int
    int int_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonInt;
        switch (found_type) {
            case expected_type: return m_value.m_int;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }
    /// Get the 64-bit float from the Value, or throw a WrongPysonType if it isn't a float
    double float_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonFloat;
        switch (found_type) {
            case expected_type: return m_value.m_float;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }
    /// Get the string from the Value, or throw a WrongPysonType if it isn't a string
    string string_or_throw() const {
        PysonType found_type = type();
        constexpr PysonType expected_type = PysonType::PysonStr;
        switch (found_type) {
            case expected_type: return m_value.m_str;
            default: throw WrongPysonType(expected_type, found_type);
        }
    }
    /// Get the list from the Value, or throw a WrongPysonType if it isn't a list
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

/// A Value, but with a name
class NamedValue {
    std::string m_name;
    Value m_value;

public:
    friend class FileReader;
    /// Print a NamedValue in the pyson format
    friend std::ostream& operator<< (std::ostream& o, NamedValue& v);
    /// Read in a NamedValue using the pyson format
    friend bool operator>> (std::istream& i, NamedValue& v);

    /// Construct a NamedValue from a name and a Value
    explicit NamedValue(const std::string& name, const Value& value) : m_name(name), m_value(value) {}
    explicit NamedValue(const std::string& name, Value&& value) : m_name(name), m_value(value) {}
    explicit NamedValue(std::string&& name, const Value& value) : m_name(name), m_value(value) {}
    explicit NamedValue(std::string&& name, Value&& value) : m_name(name), m_value(value) {}

    /// Returns the name of the NamedValue
    std::string name() const noexcept { return m_name; }
    /// Returns the Value contained by the NamedValue
    Value value() const noexcept { return m_value; }

    /// Change the name of a NamedValue
    void change_name(const std::string& new_name) noexcept { m_name = new_name; }
    /// Swap out the value of a NamedValue, keeping the name
    void change_value(const Value& new_value) noexcept { m_value = new_value; }
};

class FileReader {

#if POSIX_FUNCTIONS_AVAILABLE
    FILE *m_handle;
#else
    std::ifstream m_stream;
#endif

public:
    FileReader(const char *path);
    FileReader(const std::string& path);

    /** 
     * Get the next NamedValue from the file,
     * or the null option if either the file ended.
     * Throws an exception if the NamedValue is invalid.
     */
    std::optional<NamedValue> next();
    /**
     * Get the next NamedValue from the file,
     * or a predetermined default NamedValue if the file ended.
     * Throws an exception if the NamedValue is invalid.
     */
    NamedValue next_or(const NamedValue& default_value);
    NamedValue next_or(NamedValue&& default_value);
    /// Get the next NamedValue from the file,
    /// or throw an exception if the file ended or the NamedValue is invalid
    NamedValue next_or_throw();

    /**
     * Get a vector that contains each NamedValue from the file.
     * This call will read the entire file,
     * not just the portion after the current read position.
     */
    std::vector<NamedValue> all();

    /**
     * Get a hashmap of each name to its Value from the file.
     * This call will read the entire file,
     * not just the portion after the current read position.
     */
    std::unordered_map<std::string, Value> as_hashmap();

    /// Reset read progress to the beginning of the file
    void go_to_beginning();

    /// Go to a specific line in the file, start reading from that line
    /// Throws an exception if EOF is encountered before that line
    void go_to_line(size_t line_number);

    /// Skip the next N lines
    void skip_n_lines(size_t amount_to_skip);

    /**
     * Locate the Value with a specific name from the file.
     * The value will be found if it exists anywhere in the file,
     * even before the current read location.
     * The next Value read by next() will be the one after the value found.
     */
    std::optional<Value> value_with_name(const char *name);
    std::optional<Value> value_with_name(const std::string& name) { return value_with_name(name.c_str()); }

    /**
     * Execute a function for each NamedValue left in the file.
     * This function will not rewind to the beginning of the file.
     */
    void for_each(std::function<void(NamedValue)> predicate);

    /**
     * Map each NamedValue, and then get all of the results.
     * This function will not rewind to the beginning of the file.
     */
    template <class Return>
    std::vector<Return> map_each(std::function<Return(NamedValue)> predicate);

    /**
     * Call a function for each NamedValue left in the file while the function returns true.
     * This function will not rewind to the beginning of the file.
     */
    void for_each_while(std::function<bool(NamedValue)> predicate);

    /**
     * Map each NamedValue while the function returns true and some Return value.
     * Note: the last Return value will not be included in the result.
     * This function will not rewind to the beginning of the file.
     */
    template <class Return>
    std::vector<Return> map_while(std::function<std::pair<bool, Return>(NamedValue)> predicate);

    /**
     * Map each NamedValue while the function doesn't return std::nullopt.
     * This function will not rewind to the beginning of the file.
     */
    template <class Return>
    std::vector<Return> map_while(std::function<std::optional<Return>(NamedValue)> predicate);

    /**
     * Functions and classes that allow FileReader to be used as an iterator.
     * You should not ever use these, just do `for (NamedPysonValue value : reader) { ... }`
     * where `reader` is a FileReader. Range-based for loops like that are much more understantable
     * and less bug prone than the old iterator-based for loops, so use them.
     * FileReader::begin() will NOT rewind the file despite the name.
     */

    /// Dummy class required because != is a binary operator
    class End {};
    
    /// Iterator over the NamedValue values in the file
    class Iter {
        /// Pointer to the inner reader
        FileReader *m_reader;
        /// Cached next value
        NamedValue m_cached;
    
        /// Private constructor (only for use by FileReader)
        Iter(FileReader *reader);
    
        /// So a FileReader can construct an Iter
        friend class FileReader;

    public:
        /// Increment: go to next iteration
        Iter operator++();
        /// Dereference: get the value
        NamedValue operator*();
        /// Not equal: check if the end has been reached
        bool operator!=(const End& end);
    };

    /// Begin iterator
    Iter begin();
    /// End value
    End end();
};

}

#endif