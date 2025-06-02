
#ifndef ITMOSCRIPT_VALUE_H
#define ITMOSCRIPT_VALUE_H

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace itmoscript {

class Environment;

class Value {
   public:
    enum class Type { Number, String, Boolean, Nil, List, Function };

    using ListType = std::vector<Value>;
    using FuncType =
        std::function<Value(const std::vector<Value>&, Environment&)>;

   private:
    Type type_;
    std::variant<std::monostate, double, std::string, bool, ListType, FuncType>
        data_;

   public:
    static Value makeNumber(double x) { return Value(x); }
    static Value makeString(std::string s) { return Value(std::move(s)); }
    static Value makeBoolean(bool b) { return Value(b); }
    static Value makeNil() { return Value(); }
    static Value makeList(ListType v) { return Value(std::move(v)); }
    static Value makeFunction(FuncType f) { return Value(std::move(f)); }

    Type type() const noexcept { return type_; }
    double asNumber() const;
    const std::string& asString() const;
    bool asBoolean() const;
    const ListType& asList() const;
    const FuncType& asFunction() const;

    std::string toString() const;

    Value() noexcept;
    explicit Value(double x);
    explicit Value(const std::string& s);
    explicit Value(bool b);
    explicit Value(ListType v);
    explicit Value(FuncType f);
};

}  // namespace itmoscript

#endif
