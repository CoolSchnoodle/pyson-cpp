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
        const PysonType type = PysonType::PysonInt;
        int value;
        PysonInt() : value(0) {} 
        PysonInt(int value) : value(value) {}
    };

    struct PysonFloat {
        const PysonType type = PysonType::PysonFloat;
        double value;
        PysonFloat() : value(0.0) {}
        PysonFloat(float value) : value(value) {}
    };

    struct PysonStr {
        const PysonType type = PysonType::PysonStr;
        std::string value;
        PysonStr() : value("") {}
        PysonStr(const std::string& value) : value(value){}
        PysonStr(std::string&& value) : value(value){}
    };

    struct PysonStrList {
        const PysonType type = PysonType::PysonStrList;
        std::vector<std::string> value;
        PysonStrList() : value(std::vector<std::string>()) {}
        PysonStrList(const std::vector<std::string>& value) {}
        PysonStrList(std::vector<std::string>&& value) {}
    };


    PysonInt int_value;
    PysonFloat float_value;
    PysonStr str_value;
    PysonStrList str_list_value;

public:

    friend std::ostream& operator<< (std::ostream& o, const PysonValue& val);

    PysonValue(int int_value) : int_value(int_value) {}
    PysonValue(double float_value) : float_value(float_value) {}
    PysonValue(const std::string& str_value) : str_value(str_value) {}
    PysonValue(std::string&& str_value) : str_value(str_value) {}
    PysonValue(const std::vector<std::string>& str_list_value) : str_list_value(str_list_value) {}
    PysonValue(std::vector<std::string>&& str_list_value) : str_list_value(str_list_value) {}

    ~PysonValue();

};

#endif