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
    PysonList,
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
    PysonType m_type;
    PysonValueInner m_value;

public:
    friend class NamedPysonValue;
        friend bool operator>> (std::istream& i, NamedPysonValue& v);
    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    /// Get the type of the PysonValue as a PysonType
    PysonType type() const noexcept { return m_type; }
    /// Get the type of the PysonValue as a C string (const char *)
    const char *type_cstring() const noexcept;
    /// Get the type of the PysonValue as a C++ string (std::string)
    std::string type_string() const noexcept { return this->type_cstring(); }

    /// Returns whether the PysonValue is an integer
    bool is_int() const noexcept { return this->type() == PysonType::PysonInt; }
    /// Returns whether the PysonValue is a floating-point number
    bool is_float() const noexcept { return this->type() == PysonType::PysonFloat; }
    /// Returns whether the PysonValue is a string
    bool is_str() const noexcept { return this->type() == PysonType::PysonStr; }
    /// Returns whether the PysonValue is a list of strings
    bool is_list() const noexcept { return this->type() == PysonType::PysonList; }

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
    explicit PysonValue(int val) : m_type(PysonType::PysonInt), m_value(val) {}
    /// Construct a PysonValue from a 64-bit floating-point number
    explicit PysonValue(double val) : m_type(PysonType::PysonFloat), m_value(val) {}
    /// Construct a PysonValue from a string
    explicit PysonValue(const std::string& str) : m_type(PysonType::PysonStr), m_value(str) {}
    explicit PysonValue(std::string&& str) : m_type(PysonType::PysonStr), m_value(str) {}
    /// Construct a PysonValue from a list of strings
    explicit PysonValue(const std::vector<std::string>& list) : m_type(PysonType::PysonList), m_value(list) {}
    explicit PysonValue(std::vector<std::string>&& list) : m_type(PysonType::PysonList), m_value(list) {}

    /// Destruct a PysonValue, including correctly destructing the PysonValueInner
    ~PysonValue() noexcept;
};

/// A PysonValue, but with a name
class NamedPysonValue {
    std::string m_name;
    PysonValue m_value;

public:
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
    std::string get_name() const noexcept { return m_name; }
    /// Returns the PysonValue contained by the NamedPysonValue
    PysonValue get_value() const noexcept { return m_value; }

    /// Change the name of a NamedPysonValue
    void change_name(const std::string& new_name) noexcept { m_name = new_name; }
    /// Swap out the value of a NamedPysonValue, keeping the name
    void change_value(const PysonValue& new_value) noexcept { m_value = new_value; }
};

#endif