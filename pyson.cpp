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
    PysonType type;
    int value;
    PysonInt() : value(0), type(PysonType::PysonInt) {} 
    PysonInt(int value) : value(value), type(PysonType::PysonInt) {}
};

struct PysonFloat {
    PysonType type;
    float value;
    PysonFloat() : value(0.0), type(PysonType::PysonFloat) {}
    PysonFloat(float value) : value(value), type(PysonType::PysonFloat) {}
};

struct PysonStr {
    PysonType type;
    std::string value;
    PysonStr() : value(""), type(PysonType::PysonStr) {}
    PysonStr(const std::string& value) : value(value), type(PysonType::PysonStr) {}
    PysonStr(std::string&& value) : value(value), type(PysonType::PysonStr) {}
};

struct PysonStrList {
    PysonType type;
    std::vector<std::string> value;
    PysonStrList() : value(std::vector<std::string>()), type(PysonType::PysonStrList) {}
    PysonStrList(const std::vector<std::string>& value) : value(value), type(PysonType::PysonStrList) {}
    PysonStrList(std::vector<std::string>&& value) : value(value), type(PysonType::PysonStrList) {}
};

union PysonValue {

    PysonInt int_value;
    PysonFloat float_value;
    PysonStr str_value;
    PysonStrList str_list_value;

    PysonValue(int int_value) : int_value(int_value) {}
    PysonValue(float float_value) : float_value(float_value) {}
    PysonValue(const std::string& str_value) : str_value(str_value) {}
    PysonValue(std::string&& str_value) : str_value(str_value) {}
    PysonValue(const std::vector<std::string>& str_list_value) : str_list_value(str_list_value) {}
    PysonValue(std::vector<std::string>&& str_list_value) : str_list_value(str_list_value) {}

    ~PysonValue() {

        // because they start with the `type` field, it doesn't matter which is extracted
        switch (this->int_value.type) {
            case PysonType::PysonInt:
            case PysonType::PysonFloat:
                break;
            case PysonType::PysonStr:
                this->str_value.value.~basic_string();
                break;
            case PysonType::PysonStrList:
                this->str_list_value.value.~vector();
                break;
        }
        
    }

};