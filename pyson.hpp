#ifndef PYSON_CPP_PYSON_INCLUDED
#define PYSON_CPP_PYSON_INCLUDED

#include <string>
#include <vector>
#include <iostream>

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
        PysonType type;
        int value;
        PysonInt() : value(0), type(PysonType::PysonInt) {} 
        PysonInt(int value) : value(value), type(PysonType::PysonInt) {}
    };

    struct PysonFloat {
        PysonType type = PysonType::PysonFloat;
        float value;
        PysonFloat() : value(0.0) {}
        PysonFloat(float value) : value(value) {}
    };

    struct PysonStr {
        PysonType type = PysonType::PysonStr;
        std::string value;
        PysonStr() : value("") {}
        PysonStr(const std::string& value) : value(value){}
        PysonStr(std::string&& value) : value(value){}
    };

    struct PysonStrList {
        PysonType type;
        std::vector<std::string> value;
        PysonStrList() : value(std::vector<std::string>()), type(PysonType::PysonStrList) {}
        PysonStrList(const std::vector<std::string>& value) : value(value), type(PysonType::PysonStrList) {}
        PysonStrList(std::vector<std::string>&& value) : value(value), type(PysonType::PysonStrList) {}
    };


    PysonInt int_value;
    PysonFloat float_value;
    PysonStr str_value;
    PysonStrList str_list_value;

public:

    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    PysonValue(int int_value) : int_value(int_value) {}
    PysonValue(float float_value) : float_value(float_value) {}
    PysonValue(const std::string& str_value) : str_value(str_value) {}
    PysonValue(std::string&& str_value) : str_value(str_value) {}
    PysonValue(const std::vector<std::string>& str_list_value) : str_list_value(str_list_value) {}
    PysonValue(std::vector<std::string>&& str_list_value) : str_list_value(str_list_value) {}

    ~PysonValue();

};

#endif