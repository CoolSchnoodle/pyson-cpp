#include <string>
#include <vector>
#include <optional>

enum class PysonType {
    PysonInt,
    PysonFloat,
    PysonStr,
    PysonStrList,
};

struct PysonInt {
    int value;
    PysonType type;
    PysonInt() : value(0), type(PysonType::PysonInt) {} 
    PysonInt(int value) : value(value), type(PysonType::PysonInt) {}
};

struct PysonFloat {
    float value;
    PysonType type;
    PysonFloat() : value(0.0), type(PysonType::PysonFloat) {}
    PysonFloat(float value) : value(value), type(PysonType::PysonFloat) {}
};

struct PysonStr {
    std::string value;
    PysonType type;
    PysonStr() : value(""), type(PysonType::PysonStr) {}
    PysonStr(std::string value) : value(value), type(PysonType::PysonStr) {}
    PysonStr(std::string&& value) : value(value), type(PysonType::PysonStr) {}
};

struct PysonStrList {
    std::vector<std::string> value;
    PysonType type;
    PysonStrList() : value(std::vector<std::string>()), type(PysonType::PysonStrList) {}
    PysonStrList(std::vector<std::string> value) : value(value), type(PysonType::PysonStrList) {}
    PysonStrList(std::vector<std::string>&& value) : value(value), type(PysonType::PysonStrList) {}
};

union PysonValue {

    PysonInt int_value;
    PysonFloat float_value;
    PysonStr str_value;
    PysonStrList str_list_value;

    PysonValue(int int_value) : int_value(int_value) {}
    PysonValue(float float_value) : float_value(float_value) {}
    PysonValue(std::string str_value) : str_value(str_value) {}
    PysonValue(std::string&& str_value) : str_value(str_value) {}
    PysonValue(std::vector<std::string> str_list_value) : str_list_value(str_list_value) {}
    PysonValue(std::vector<std::string>&& str_list_value) : str_list_value(str_list_value) {}

};