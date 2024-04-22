#ifndef PYSON_CPP_PYSON_INCLUDED
#define PYSON_CPP_PYSON_INCLUDED

#include <string>
#include <vector>
#include <iosfwd>

class PysonValue;
class NamedPysonValue;

enum class PysonType {
    PysonInt,
    PysonFloat,
    PysonStr,
    PysonStrList,
};

union PysonValueInner {

    friend class PysonValue;
        friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);
    friend class NamedPysonValue;

private:

    int int_value;
    double float_value;
    std::string str_value;
    std::vector<std::string> str_list_value;

    PysonValueInner(int val) noexcept : int_value(val) {}
    PysonValueInner(double val) noexcept : float_value(val) {}
    PysonValueInner(const std::string& str) noexcept : str_value(str) {}
    PysonValueInner(std::string&& str) noexcept : str_value(str) {}
    PysonValueInner(const std::vector<std::string>& list) noexcept : str_list_value(list) {}
    PysonValueInner(std::vector<std::string>&& list) noexcept : str_list_value(list) {}

    // You don't get a PysonValueInner, only a PysonValue can get one and their destructor takes care of it
    ~PysonValueInner() noexcept {}

};

class PysonValue {

    PysonType type;
    PysonValueInner value;

public:

    friend class NamedPysonValue;
        friend bool operator>> (std::istream& i, NamedPysonValue& v);

    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    PysonType get_type() const noexcept { return this->type; }
    const char *get_type_cstring() const noexcept;
    std::string get_type_string() const noexcept { return this->get_type_cstring(); }

    bool is_int() const noexcept { return this->get_type() == PysonType::PysonInt; }
    bool is_float() const noexcept { return this->get_type() == PysonType::PysonFloat; }
    bool is_str() const noexcept { return this->get_type() == PysonType::PysonStr; }
    bool is_str_list() const noexcept { return this->get_type() == PysonType::PysonStrList; }

    std::string value_as_string() const noexcept;

    PysonValue(const PysonValue&);
    PysonValue(PysonValue&&);
    PysonValue& operator= (const PysonValue&);
    PysonValue& operator= (PysonValue&&);

    static PysonValue from_pyson_list(std::string pyson_list);
    explicit PysonValue(int val) : type(PysonType::PysonInt), value(val) {}
    explicit PysonValue(double val) : type(PysonType::PysonFloat), value(val) {}
    explicit PysonValue(const std::string& str) : type(PysonType::PysonStr), value(str) {}
    explicit PysonValue(std::string&& str) : type(PysonType::PysonStr), value(str) {}
    explicit PysonValue(const std::vector<std::string>& list) : type(PysonType::PysonStrList), value(list) {}
    explicit PysonValue(std::vector<std::string>&& list) : type(PysonType::PysonStrList), value(list) {}

    ~PysonValue() noexcept;


};

class NamedPysonValue {
    
    std::string name;
    PysonValue value;

public:

    friend std::ostream& operator<< (std::ostream& o, NamedPysonValue& v);
    friend bool operator>> (std::istream& i, NamedPysonValue& v);

    explicit NamedPysonValue(const std::string& name, const PysonValue& value) : name(name), value(value) {}
    explicit NamedPysonValue(const std::string& name, PysonValue&& value) : name(name), value(value) {}
    explicit NamedPysonValue(std::string&& name, const PysonValue& value) : name(name), value(value) {}
    explicit NamedPysonValue(std::string&& name, PysonValue&& value) : name(name), value(value) {}

    std::string get_name() const noexcept { return this->name; }
    PysonValue get_value() const noexcept { return this->value; }

    void change_name(const std::string& new_name) noexcept { this->name = new_name; }
    void change_value(const PysonValue& new_value) noexcept { this->value = new_value; }

};

#endif