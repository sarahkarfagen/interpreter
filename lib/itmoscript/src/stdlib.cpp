#include "itmoscript/stdlib.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "itmoscript/value.h"

namespace itmoscript {

void registerStandardLibrary(Environment::Builder& eb) {
    eb.addGlobal("print", Value::makeFunction(
                              [](auto const& args, Environment& env) -> Value {
                                  for (const auto& v : args) {
                                      if (v.type() == Value::Type::String) {
                                          const auto& s = v.asString();

                                          env.out() << s;
                                      } else {
                                          env.out() << v.toString();
                                      }
                                  }
                                  return Value::makeNil();
                              }));

    eb.addGlobal("println", Value::makeFunction([](auto const& args,
                                                   Environment& env) -> Value {
                     for (const auto& v : args) {
                         if (v.type() == Value::Type::String) {
                             const auto& s = v.asString();
                             env.out() << s;
                         } else {
                             env.out() << v.toString();
                         }
                     }
                     env.out() << "\n";
                     return Value::makeNil();
                 }));

    eb.addGlobal("read", Value::makeFunction([](auto const& args,
                                                Environment& env) -> Value {
                     if (!args.empty()) {
                         throw std::runtime_error("read expects 0 args");
                     }
                     std::string line;
                     if (!std::getline(env.in(), line)) {
                         return Value::makeNil();
                     }
                     return Value::makeString(line);
                 }));

    eb.addGlobal(
        "stacktrace",
        Value::makeFunction([](auto const& args, Environment& env) -> Value {
            if (!args.empty()) {
                throw std::runtime_error("stacktrace expects 0 args");
            }
            const auto& st = env.getCallStack();
            std::vector<Value> outList;
            outList.reserve(st.size());
            for (const auto& fnName : st) {
                outList.push_back(Value::makeString(fnName));
            }
            return Value::makeList(std::move(outList));
        }));

    eb.addGlobal("range", Value::makeFunction([](auto const& args,
                                                 Environment&) -> Value {
                     if (args.size() != 3)
                         throw std::runtime_error("range expects 3 args");
                     int a = static_cast<int>(args[0].asNumber());
                     int b = static_cast<int>(args[1].asNumber());
                     int step = static_cast<int>(args[2].asNumber());
                     if (step == 0) throw std::runtime_error("range step zero");
                     std::vector<Value> outList;
                     for (int i = a; (step > 0 ? i < b : i > b); i += step) {
                         outList.push_back(Value::makeNumber(i));
                     }
                     return Value::makeList(std::move(outList));
                 }));

    eb.addGlobal(
        "len", Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 1) throw std::runtime_error("len expects 1 arg");
            if (args[0].type() == Value::Type::String) {
                return Value::makeNumber(args[0].asString().size());
            } else if (args[0].type() == Value::Type::List) {
                return Value::makeNumber(args[0].asList().size());
            }
            throw std::runtime_error("len unsupported type");
        }));

    eb.addGlobal(
        "abs", Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 1) throw std::runtime_error("abs expects 1 arg");
            double x = args[0].asNumber();
            return Value::makeNumber(std::fabs(x));
        }));

    eb.addGlobal("ceil", Value::makeFunction([](auto const& args,
                                                Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("ceil expects 1 arg");
                     double x = args[0].asNumber();
                     return Value::makeNumber(std::ceil(x));
                 }));

    eb.addGlobal("floor", Value::makeFunction([](auto const& args,
                                                 Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("floor expects 1 arg");
                     double x = args[0].asNumber();
                     return Value::makeNumber(std::floor(x));
                 }));

    eb.addGlobal("round", Value::makeFunction([](auto const& args,
                                                 Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("round expects 1 arg");
                     double x = args[0].asNumber();
                     return Value::makeNumber(std::round(x));
                 }));

    eb.addGlobal("sqrt", Value::makeFunction([](auto const& args,
                                                Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("sqrt expects 1 arg");
                     double x = args[0].asNumber();
                     if (x < 0) throw std::runtime_error("sqrt of negative");
                     return Value::makeNumber(std::sqrt(x));
                 }));

    eb.addGlobal(
        "rnd", Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 1) throw std::runtime_error("rnd expects 1 arg");
            int n = static_cast<int>(args[0].asNumber());
            if (n <= 0) throw std::runtime_error("rnd argument must be > 0");

            static std::mt19937_64 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, n - 1);
            return Value::makeNumber(dist(gen));
        }));

    eb.addGlobal("parse_num", Value::makeFunction([](auto const& args,
                                                     Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("parse_num expects 1 arg");
                     const std::string& s = args[0].asString();
                     try {
                         size_t idx = 0;
                         double d = std::stod(s, &idx);
                         if (idx == s.size()) {
                             return Value::makeNumber(d);
                         }
                     } catch (...) {
                     }
                     return Value::makeNil();
                 }));

    eb.addGlobal("to_string", Value::makeFunction([](auto const& args,
                                                     Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("to_string expects 1 arg");
                     double x = args[0].asNumber();
                     if (std::floor(x) == x) {
                         return Value::makeString(
                             std::to_string(static_cast<long long>(x)));
                     } else {
                         return Value::makeString(std::to_string(x));
                     }
                 }));

    eb.addGlobal("lower", Value::makeFunction([](auto const& args,
                                                 Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("lower expects 1 arg");
                     std::string s = args[0].asString();
                     for (char& c : s) {
                         c = static_cast<char>(
                             std::tolower(static_cast<unsigned char>(c)));
                     }
                     return Value::makeString(std::move(s));
                 }));

    eb.addGlobal("upper", Value::makeFunction([](auto const& args,
                                                 Environment&) -> Value {
                     if (args.size() != 1)
                         throw std::runtime_error("upper expects 1 arg");
                     std::string s = args[0].asString();
                     for (char& c : s) {
                         c = static_cast<char>(
                             std::toupper(static_cast<unsigned char>(c)));
                     }
                     return Value::makeString(std::move(s));
                 }));

    eb.addGlobal(
        "split",
        Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("split expects 2 args");
            const std::string& s = args[0].asString();
            const std::string& delim = args[1].asString();
            std::vector<Value> parts;

            if (delim.empty()) {
                for (char c : s) {
                    parts.push_back(Value::makeString(std::string(1, c)));
                }
            } else {
                size_t start = 0, pos;
                while ((pos = s.find(delim, start)) != std::string::npos) {
                    parts.push_back(
                        Value::makeString(s.substr(start, pos - start)));
                    start = pos + delim.size();
                }

                parts.push_back(Value::makeString(s.substr(start)));
            }
            return Value::makeList(std::move(parts));
        }));

    eb.addGlobal(
        "join",
        Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("join expects 2 args");
            const auto& lstVal = args[0];
            if (lstVal.type() != Value::Type::List)
                throw std::runtime_error("join first arg must be a list");
            const std::string& delim = args[1].asString();

            const auto& listRef = lstVal.asList();
            std::ostringstream oss;
            for (size_t i = 0; i < listRef.size(); ++i) {
                if (listRef[i].type() != Value::Type::String)
                    throw std::runtime_error(
                        "join only supports lists of strings");
                oss << listRef[i].asString();
                if (i + 1 < listRef.size()) {
                    oss << delim;
                }
            }
            return Value::makeString(oss.str());
        }));

    eb.addGlobal("replace", Value::makeFunction([](auto const& args,
                                                   Environment&) -> Value {
                     if (args.size() != 3)
                         throw std::runtime_error("replace expects 3 args");
                     std::string s = args[0].asString();
                     const std::string& oldSub = args[1].asString();
                     const std::string& newSub = args[2].asString();
                     if (oldSub.empty()) {
                         return Value::makeString(s);
                     }
                     size_t pos = 0;
                     while ((pos = s.find(oldSub, pos)) != std::string::npos) {
                         s.replace(pos, oldSub.size(), newSub);
                         pos += newSub.size();
                     }
                     return Value::makeString(std::move(s));
                 }));

    eb.addGlobal(
        "push",
        Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("push expects 2 args");
            if (args[0].type() != Value::Type::List)
                throw std::runtime_error("push first arg must be a list");
            auto newList = args[0].asList();
            newList.push_back(args[1]);
            return Value::makeList(std::move(newList));
        }));

    eb.addGlobal(
        "pop", Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 1) throw std::runtime_error("pop expects 1 arg");
            if (args[0].type() != Value::Type::List)
                throw std::runtime_error("pop arg must be a list");
            auto copyList = args[0].asList();
            if (copyList.empty()) throw std::runtime_error("pop on empty list");
            Value last = copyList.back();
            return last;
        }));

    eb.addGlobal(
        "insert",
        Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 3)
                throw std::runtime_error("insert expects 3 args");
            if (args[0].type() != Value::Type::List)
                throw std::runtime_error("insert first arg must be a list");
            int idx = static_cast<int>(args[1].asNumber());
            auto newList = args[0].asList();
            if (idx < 0 || idx > static_cast<int>(newList.size()))
                throw std::runtime_error("insert index out of bounds");
            newList.insert(newList.begin() + idx, args[2]);
            return Value::makeList(std::move(newList));
        }));

    eb.addGlobal(
        "remove",
        Value::makeFunction([](auto const& args, Environment&) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("remove expects 2 args");
            if (args[0].type() != Value::Type::List)
                throw std::runtime_error("remove first arg must be a list");
            int idx = static_cast<int>(args[1].asNumber());
            auto newList = args[0].asList();
            if (idx < 0 || idx >= static_cast<int>(newList.size()))
                throw std::runtime_error("remove index out of bounds");
            newList.erase(newList.begin() + idx);
            return Value::makeList(std::move(newList));
        }));

    eb.addGlobal(
        "sort",
        Value::makeFunction([](auto const& args, Environment& env) -> Value {
            if (args.size() < 1 || args.size() > 2) {
                throw std::runtime_error("sort expects 1 or 2 args");
            }

            if (args[0].type() != Value::Type::List) {
                throw std::runtime_error("sort first arg must be a list");
            }

            auto newList = args[0].asList();

            if (args.size() == 1) {
                std::stable_sort(newList.begin(), newList.end(),
                                 [](const Value& a, const Value& b) {
                                     return a.toString() < b.toString();
                                 });
            } else {
                if (args[1].type() != Value::Type::Function) {
                    throw std::runtime_error(
                        "sort second arg must be a function");
                }

                auto cmpFunc = args[1].asFunction();

                std::stable_sort(
                    newList.begin(), newList.end(),
                    [&](const Value& a, const Value& b) {
                        Value result = cmpFunc({a, b}, env);
                        if (result.type() != Value::Type::Boolean) {
                            throw std::runtime_error(
                                "sort comparator must return boolean");
                        }
                        return result.asBoolean();
                    });
            }

            return Value::makeList(std::move(newList));
        }));
}

}  // namespace itmoscript
