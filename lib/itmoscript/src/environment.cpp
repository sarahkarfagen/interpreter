
#include "itmoscript/environment.h"

#include <stdexcept>

namespace itmoscript {

Value Environment::get(const std::string& name) const {
    for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }

    auto g = globals_.find(name);
    if (g != globals_.end()) {
        return g->second;
    }
    throw std::runtime_error("Undefined variable '" + name + "'");
}

void Environment::set(const std::string& name, Value val) {
    for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
        if (it->count(name)) {
            (*it)[name] = std::move(val);
            return;
        }
    }

    frames_.back().emplace(name, std::move(val));
}

void Environment::pushFrame() { frames_.emplace_back(); }

void Environment::popFrame() {
    if (frames_.size() > 1) {
        frames_.pop_back();
    }
}

}  // namespace itmoscript
