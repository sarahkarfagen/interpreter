
#ifndef ITMOSCRIPT_ENVIRONMENT_H
#define ITMOSCRIPT_ENVIRONMENT_H

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

}  // namespace itmoscript

#endif
