#ifndef PYSON_CPP_PYSON_INCLUDED
#define PYSON_CPP_PYSON_INCLUDED

#include <string>
#include <vector>
#include <iosfwd>

class PysonValue;
class NamedPysonValue;

/**
 * An enum that says which type a PysonValue is.
 * Values in pyson can either be integers, 64-bit floating point numbers, strings, or lists of strings.
 */
enum class PysonType {
    PysonInt,
    PysonFloat,
    PysonStr,
    PysonStrList,
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

    int int_value;
    double float_value;
    std::string str_value;
    std::vector<std::string> str_list_value;

    /// Users of pyson can't get a PysonValueInner, so the PysonValueInner constructors are private.
    /// Only a PysonValue can get one and their destructor takes care of destructing the correct way.

    PysonValueInner(int val) noexcept : int_value(val) {}
    PysonValueInner(double val) noexcept : float_value(val) {}
    PysonValueInner(const std::string& str) noexcept : str_value(str) {}
    PysonValueInner(std::string&& str) noexcept : str_value(str) {}
    PysonValueInner(const std::vector<std::string>& list) noexcept : str_list_value(list) {}
    PysonValueInner(std::vector<std::string>&& list) noexcept : str_list_value(list) {}

    ~PysonValueInner() noexcept {}

};

/**
 * A value from a pyson file.
 * Structurally, contains a PysonValueInner union and a PysonType tag.
 */
class PysonValue {

    PysonType type;
    PysonValueInner value;

public:

    friend class NamedPysonValue;
        friend bool operator>> (std::istream& i, NamedPysonValue& v);

    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    /// Get the type of the PysonValue as a PysonType
    PysonType get_type() const noexcept { return this->type; }
    /// Get the type of the PysonValue as a C string (const char *)
    const char *get_type_cstring() const noexcept;
    /// Get the type of the PysonValue as a C++ string (std::string)
    std::string get_type_string() const noexcept { return this->get_type_cstring(); }

    /// Returns whether the PysonValue is an integer
    bool is_int() const noexcept { return this->get_type() == PysonType::PysonInt; }
    /// Returns whether the PysonValue is a floating-point number
    bool is_float() const noexcept { return this->get_type() == PysonType::PysonFloat; }
    /// Returns whether the PysonValue is a string
    bool is_str() const noexcept { return this->get_type() == PysonType::PysonStr; }
    /// Returns whether the PysonValue is a list of strings
    bool is_str_list() const noexcept { return this->get_type() == PysonType::PysonStrList; }

    // Returns the PysonValue's value as a string
    std::string value_as_string() const noexcept;

    /// Construct a PysonValue from another PysonValue
    PysonValue(const PysonValue&);
    PysonValue(PysonValue&&);
    /// Assign a PysonValue to the value of another PysonValue
    PysonValue& operator= (const PysonValue&);
    PysonValue& operator= (PysonValue&&);

    /// Construct a PysonValue from a string formatted as a pyson list
    static PysonValue from_pyson_list(std::string pyson_list);

    /// Construct a PysonValue from an integer
    explicit PysonValue(int val) : type(PysonType::PysonInt), value(val) {}
    /// Construct a PysonValue from a 64-bit floating-point number
    explicit PysonValue(double val) : type(PysonType::PysonFloat), value(val) {}
    /// Construct a PysonValue from a string
    explicit PysonValue(const std::string& str) : type(PysonType::PysonStr), value(str) {}
    explicit PysonValue(std::string&& str) : type(PysonType::PysonStr), value(str) {}
    /// Construct a PysonValue from a list of strings
    explicit PysonValue(const std::vector<std::string>& list) : type(PysonType::PysonStrList), value(list) {}
    explicit PysonValue(std::vector<std::string>&& list) : type(PysonType::PysonStrList), value(list) {}

    /// Destruct a PysonValue, including correctly destructing the PysonValueInner
    ~PysonValue() noexcept;

};

/// A PysonValue, but with a name
class NamedPysonValue {
    
    std::string name;
    PysonValue value;

public:

    /// Print a NamedPysonValue in the pyson format
    friend std::ostream& operator<< (std::ostream& o, NamedPysonValue& v);
    /// Read in a NamedPysonValue using the pyson format
    friend bool operator>> (std::istream& i, NamedPysonValue& v);

    /// Construct a NamedPysonValue from a name and a PysonValue
    explicit NamedPysonValue(const std::string& name, const PysonValue& value) : name(name), value(value) {}
    explicit NamedPysonValue(const std::string& name, PysonValue&& value) : name(name), value(value) {}
    explicit NamedPysonValue(std::string&& name, const PysonValue& value) : name(name), value(value) {}
    explicit NamedPysonValue(std::string&& name, PysonValue&& value) : name(name), value(value) {}

    /// Returns the name of the NamedPysonValue
    std::string get_name() const noexcept { return this->name; }
    /// Returns the PysonValue contained by the NamedPysonValue
    PysonValue get_value() const noexcept { return this->value; }

    /// Change the name of a NamedPysonValue
    void change_name(const std::string& new_name) noexcept { this->name = new_name; }
    /// Swap out the value of a NamedPysonValue, keeping the name
    void change_value(const PysonValue& new_value) noexcept { this->value = new_value; }

};

#endif