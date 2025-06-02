
#include "itmoscript/value.h"

#include <cmath>
#include <stdexcept>

namespace itmoscript {

Value::Value() noexcept : type_(Type::Nil), data_(std::monostate{}) {}

Value::Value(double x) : type_(Type::Number), data_(x) {}

Value::Value(const std::string& s) : type_(Type::String), data_(s) {}

Value::Value(bool b) : type_(Type::Boolean), data_(b) {}

Value::Value(ListType v) : type_(Type::List), data_(std::move(v)) {}

Value::Value(FuncType f) : type_(Type::Function), data_(std::move(f)) {}

double Value::asNumber() const {
    if (type_ != Type::Number) throw std::runtime_error("Not a number");
    return std::get<double>(data_);
}

const std::string& Value::asString() const {
    if (type_ != Type::String) throw std::runtime_error("Not a string");
    return std::get<std::string>(data_);
}

bool Value::asBoolean() const {
    if (type_ != Type::Boolean) throw std::runtime_error("Not a boolean");
    return std::get<bool>(data_);
}

const Value::ListType& Value::asList() const {
    if (type_ != Type::List) throw std::runtime_error("Not a list");
    return std::get<ListType>(data_);
}

const Value::FuncType& Value::asFunction() const {
    if (type_ != Type::Function) throw std::runtime_error("Not a function");
    return std::get<FuncType>(data_);
}

std::string Value::toString() const {
    switch (type_) {
        case Type::Number: {
            double v = asNumber();
            if (std::floor(v) == v) {
                return std::to_string((long long)v);
            } else {
                return std::to_string(v);
            }
        }
        case Type::String:
            return asString();
        case Type::Boolean:
            return asBoolean() ? "true" : "false";
        case Type::Nil:
            return "nil";
        case Type::List: {
            const auto& lst = asList();
            std::string out = "[";
            for (size_t i = 0; i < lst.size(); ++i) {
                out += lst[i].toString();
                if (i + 1 < lst.size()) out += ", ";
            }
            out += "]";
            return out;
        }
        case Type::Function:
            return "<function>";
    }
    return "";
}

}  // namespace itmoscript
