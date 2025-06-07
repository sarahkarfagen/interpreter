#ifndef ITMOSCRIPT_ENVIRONMENT_H
#define ITMOSCRIPT_ENVIRONMENT_H

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "itmoscript/value.h"

namespace itmoscript {

class Environment {
   public:
    class Builder;

    Value get(const std::string& name) const;

    void set(const std::string& name, Value val);

    void pushFrame();
    void popFrame();

    std::ostream& out() const noexcept { return *out_; }

    std::istream& in() const noexcept { return *in_; }

    void pushStack(const std::string& fnName) { callStack_.push_back(fnName); }
    void popStack() {
        if (!callStack_.empty()) callStack_.pop_back();
    }

    const std::vector<std::string>& getCallStack() const noexcept {
        return callStack_;
    }

   private:
    std::vector<std::unordered_map<std::string, Value>> frames_;
    std::unordered_map<std::string, Value> globals_;
    std::ostream* out_ = nullptr;
    std::istream* in_ = nullptr;

    std::vector<std::string> callStack_;

    friend class Builder;

   public:
    const std::unordered_map<std::string, Value>& getLocals() const noexcept {
        return frames_.back();
    }
};

class Environment::Builder {
    std::unordered_map<std::string, Value> globals_;
    std::ostream* out_ = nullptr;
    std::istream* in_ = nullptr;

   public:
    Builder& addGlobal(std::string name, Value val) {
        globals_.emplace(std::move(name), std::move(val));
        return *this;
    }

    Builder& setOutput(std::ostream& os) {
        out_ = &os;
        return *this;
    }

    Builder& setInput(std::istream& is) {
        in_ = &is;
        return *this;
    }

    std::unique_ptr<Environment> build() {
        auto env = std::make_unique<Environment>();
        env->globals_ = std::move(globals_);
        env->frames_.push_back({});
        env->out_ = out_;
        env->in_ = in_;
        return env;
    }
};

}  // namespace itmoscript

#endif
