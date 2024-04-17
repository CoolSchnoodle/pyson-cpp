#ifndef PYSON_CPP_PYSON_INCLUDED
#define PYSON_CPP_PYSON_INCLUDED

#include <string>
#include <vector>
#include <iostream>

class NamedPysonValue;

union PysonValue {

public:

    enum class PysonType {
        PysonInt,
        PysonFloat,
        PysonStr,
        PysonStrList,
    };

private:

    struct PysonInt {
        const PysonType type = PysonType::PysonInt;
        int value;
        PysonInt(int value) : value(value) {}
    };

    struct PysonFloat {
        const PysonType type = PysonType::PysonFloat;
        double value;
        PysonFloat(float value) : value(value) {}
    };

    struct PysonStr {
        const PysonType type = PysonType::PysonStr;
        std::string value;
        PysonStr(const std::string& value) : value(value){}
        PysonStr(std::string&& value) : value(value){}
    };

    struct PysonStrList {
        const PysonType type = PysonType::PysonStrList;
        std::vector<std::string> value;
        PysonStrList(const std::vector<std::string>& value) : value(value) {}
        PysonStrList(std::vector<std::string>&& value) : value(value) {}
    };


    PysonInt int_value;
    PysonFloat float_value;
    PysonStr str_value;
    PysonStrList str_list_value;

public:

    friend class NamedPysonValue;
    
    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    bool is_int() const noexcept { return this->int_value.type == PysonType::PysonInt; }
    bool is_float() const noexcept { return this->float_value.type == PysonType::PysonFloat; }
    bool is_str() const noexcept { return this->str_value.type == PysonType::PysonStr; }
    bool is_str_list() const noexcept { return this->str_list_value.type == PysonType::PysonStrList; }

    PysonType get_type() const noexcept { return this->int_value.type; }
    std::string get_type_string() const noexcept;
    const char *get_type_cstring() const noexcept;

    std::string value_as_string() const noexcept;

    PysonValue(const PysonValue&);
    PysonValue(PysonValue&&);
    PysonValue& operator= (const PysonValue&);
    PysonValue& operator= (PysonValue&&);

    explicit PysonValue(int int_value) : int_value(int_value) {}
    explicit PysonValue(double float_value) : float_value(float_value) {}
    explicit PysonValue(const std::string& str_value) : str_value(str_value) {}
    explicit PysonValue(std::string&& str_value) : str_value(str_value) {}
    explicit PysonValue(const std::vector<std::string>& str_list_value) : str_list_value(str_list_value) {}
    explicit PysonValue(std::vector<std::string>&& str_list_value) : str_list_value(str_list_value) {}

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