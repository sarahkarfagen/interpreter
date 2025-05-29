#ifndef ITMOSCRIPT_INTERPRETER_H
#define ITMOSCRIPT_INTERPRETER_H

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "itmoscript/ast.h"

namespace itmoscript {

class Environment;
class ExecNode;
using ExecNodePtr = std::unique_ptr<ExecNode>;

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

    Value() noexcept : type_(Type::Nil), data_(std::monostate{}) {}
    Value(double x) : type_(Type::Number), data_(x) {}
    Value(const std::string& s) : type_(Type::String), data_(s) {}
    Value(bool b) : type_(Type::Boolean), data_(b) {}
    Value(ListType v) : type_(Type::List), data_(std::move(v)) {}
    Value(FuncType f) : type_(Type::Function), data_(std::move(f)) {}
};

class Environment {
   public:
    class Builder;

    Value get(const std::string& name) const;
    void set(const std::string& name, Value val);

    void pushFrame();
    void popFrame();

    std::ostream& out() const noexcept { return *out_; }

   private:
    std::vector<std::unordered_map<std::string, Value>> frames_;
    std::unordered_map<std::string, Value> globals_;
    std::ostream* out_ = nullptr;

    friend class Builder;
};

class Environment::Builder {
    std::unordered_map<std::string, Value> globals_;
    std::ostream* out_ = nullptr;

   public:
    Builder& addGlobal(std::string name, Value val) {
        globals_.emplace(std::move(name), std::move(val));
        return *this;
    }
    Builder& setOutput(std::ostream& os) {
        out_ = &os;
        return *this;
    }

    std::unique_ptr<Environment> build() {
        auto env = std::make_unique<Environment>();
        env->globals_ = std::move(globals_);
        env->frames_.push_back({});
        env->out_ = out_;
        return env;
    }
};

struct ReturnException {
    Value value;
};
struct BreakException {};
struct ContinueException {};

class ExecNode {
   public:
    virtual ~ExecNode() = default;

    virtual Value execute(Environment& env) = 0;
};

ExecNodePtr buildExecTree(const ASTNode* ast);

bool interpret(std::istream& in, std::ostream& out);

}  // namespace itmoscript

#endif