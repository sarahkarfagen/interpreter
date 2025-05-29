

#include "itmoscript/interpreter.h"

#include <cmath>
#include <stdexcept>

#include "itmoscript/lexer.h"
#include "itmoscript/parser.h"

namespace itmoscript {

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
            auto v = asNumber();
            if (std::floor(v) == v)
                return std::to_string((long long)v);
            else
                return std::to_string(v);
        }
        case Type::String:
            return asString();
        case Type::Boolean:
            return asBoolean() ? "true" : "false";
        case Type::Nil:
            return "nil";
        case Type::List: {
            const auto& lst = asList();
            std::string s = "[";
            for (size_t i = 0; i < lst.size(); ++i) {
                s += lst[i].toString();
                if (i + 1 < lst.size()) s += ", ";
            }
            s += "]";
            return s;
        }
        case Type::Function:
            return "<function>";
    }
    return "";
}

Value Environment::get(const std::string& name) const {
    for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }

    auto g = globals_.find(name);
    if (g != globals_.end()) return g->second;
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
    if (frames_.size() > 1) frames_.pop_back();
}

namespace {

inline void type_error(const std::string& what) {
    throw std::runtime_error("Type error: " + what);
}

class Builder {
    const ASTNode* ast_;

   public:
    explicit Builder(const ASTNode* root) : ast_(root) {}

    ExecNodePtr build() { return buildNode(ast_); }

   private:
    ExecNodePtr buildNode(const ASTNode* node) {
        using NT = NodeType;
        switch (node->type) {
            case NT::Program:
                return makeProgram(node);
            case NT::StatementList:
                return makeStmtList(node);
            case NT::Assignment:
                return makeAssignment(node);
            case NT::FunctionCall:
                return makeFuncCall(node);
            case NT::Return:
                return makeReturn(node);
            case NT::Break:
                return makeBreak();
            case NT::Continue:
                return makeContinue();
            case NT::If:
                return makeIf(node);
            case NT::While:
                return makeWhile(node);
            case NT::For:
                return makeFor(node);
            case NT::FunctionDefinition:
                return makeLambda(node);
            case NT::BinaryOp:
                return makeBinaryOp(node);
            case NT::UnaryOp:
                return makeUnaryOp(node);
            case NT::Literal:
            case NT::Boolean:
            case NT::Nil:
                return makeLiteral(node);
            case NT::Identifier:
                return makeIdentifier(node);
            case NT::ListLiteral:
                return makeListLiteral(node);
            default:
                throw std::runtime_error("Unsupported AST node");
        }
    }

    ExecNodePtr makeProgram(const ASTNode* p) {
        return buildNode(p->children[0].get());
    }

    ExecNodePtr makeStmtList(const ASTNode* p) {
        struct SL : ExecNode {
            std::vector<ExecNodePtr> stmts;
            Value execute(Environment& env) override {
                for (auto& s : stmts) {
                    s->execute(env);
                }
                return Value::makeNil();
            }
        };
        auto out = std::make_unique<SL>();
        for (auto& c : p->children) out->stmts.push_back(buildNode(c.get()));
        return out;
    }

    ExecNodePtr makeAssignment(const ASTNode* p) {
        struct A : ExecNode {
            std::string name, op;
            ExecNodePtr expr;
            A(std::string n, std::string o, ExecNodePtr e)
                : name(std::move(n)), op(std::move(o)), expr(std::move(e)) {}
            Value execute(Environment& env) override {
                auto v = expr->execute(env);
                if (op != "=") {
                    Value old = env.get(name);

                    if (old.type() != Value::Type::Number ||
                        v.type() != Value::Type::Number) {
                        type_error(op + " requires numbers");
                    }

                    double a = old.asNumber();
                    double b = v.asNumber();
                    if (op == "+=")
                        v = Value::makeNumber(a + b);
                    else if (op == "-=")
                        v = Value::makeNumber(a - b);
                    else if (op == "*=")
                        v = Value::makeNumber(a * b);
                    else if (op == "/=")
                        v = Value::makeNumber(a / b);
                    else if (op == "%=")
                        v = Value::makeNumber(std::fmod(a, b));
                    else if (op == "^=")
                        v = Value::makeNumber(std::pow(a, b));
                    else
                        type_error("Unsupported assignment op '" + op + "'");
                }
                env.set(name, v);
                return Value::makeNil();
            }
        };
        auto var = p->value;
        auto op = p->children[1]->value;
        auto rhs = buildNode(p->children[2].get());
        return std::make_unique<A>(var, op, std::move(rhs));
    }

    ExecNodePtr makeFuncCall(const ASTNode* p) {
        struct FC : ExecNode {
            std::string fname;
            std::vector<ExecNodePtr> args;
            FC(std::string f, std::vector<ExecNodePtr> a)
                : fname(std::move(f)), args(std::move(a)) {}
            Value execute(Environment& env) override {
                auto fval = env.get(fname);
                if (fval.type() != Value::Type::Function)
                    type_error("Not a function: " + fname);
                std::vector<Value> avals;
                avals.reserve(args.size());
                for (auto& a : args) avals.push_back(a->execute(env));
                return fval.asFunction()(avals, env);
            }
        };
        std::vector<ExecNodePtr> args;
        if (!p->children.empty()) {
            for (auto& c0 : p->children[0]->children)
                args.push_back(buildNode(c0.get()));
        }
        return std::make_unique<FC>(p->value, std::move(args));
    }

    ExecNodePtr makeReturn(const ASTNode* p) {
        struct R : ExecNode {
            ExecNodePtr expr;
            R(ExecNodePtr e) : expr(std::move(e)) {}
            Value execute(Environment& env) override {
                auto v = expr->execute(env);
                throw ReturnException{v};
            }
        };
        return std::make_unique<R>(buildNode(p->children[0].get()));
    }

    ExecNodePtr makeBreak() {
        struct B : ExecNode {
            Value execute(Environment&) override { throw BreakException{}; }
        };
        return std::make_unique<B>();
    }
    ExecNodePtr makeContinue() {
        struct C : ExecNode {
            Value execute(Environment&) override { throw ContinueException{}; }
        };
        return std::make_unique<C>();
    }

    ExecNodePtr makeIf(const ASTNode* p) {
        struct I : ExecNode {
            std::vector<std::pair<ExecNodePtr, ExecNodePtr>> clauses;
            ExecNodePtr elseBody;
            Value execute(Environment& env) override {
                for (auto& [cond, body] : clauses) {
                    if (cond->execute(env).asBoolean()) {
                        return body->execute(env);
                    }
                }
                if (elseBody) return elseBody->execute(env);
                return Value::makeNil();
            }
        };
        auto out = std::make_unique<I>();

        out->clauses.emplace_back(buildNode(p->children[0].get()),
                                  buildNode(p->children[1].get()));

        for (size_t i = 2; i < p->children.size(); ++i) {
            const auto* c = p->children[i].get();
            if (c->type == NodeType::ElseIf) {
                out->clauses.emplace_back(buildNode(c->children[0].get()),
                                          buildNode(c->children[1].get()));
            } else if (c->type == NodeType::Else) {
                out->elseBody = buildNode(c->children[0].get());
            }
        }
        return out;
    }

    ExecNodePtr makeWhile(const ASTNode* p) {
        struct W : ExecNode {
            ExecNodePtr cond, body;
            W(ExecNodePtr c, ExecNodePtr b)
                : cond(std::move(c)), body(std::move(b)) {}
            Value execute(Environment& env) override {
                while (cond->execute(env).asBoolean()) {
                    try {
                        body->execute(env);
                    } catch (ContinueException&) {
                        continue;
                    } catch (BreakException&) {
                        break;
                    }
                }
                return Value::makeNil();
            }
        };
        return std::make_unique<W>(buildNode(p->children[0].get()),
                                   buildNode(p->children[1].get()));
    }

    ExecNodePtr makeFor(const ASTNode* p) {
        struct F : ExecNode {
            std::string var;
            ExecNodePtr iterable, body;
            F(std::string v, ExecNodePtr it, ExecNodePtr b)
                : var(std::move(v)),
                  iterable(std::move(it)),
                  body(std::move(b)) {}
            Value execute(Environment& env) override {
                auto col = iterable->execute(env);
                if (col.type() != Value::Type::List)
                    type_error("For loop expects list");
                const auto& lst = col.asList();
                for (auto& elt : lst) {
                    env.pushFrame();
                    env.set(var, elt);
                    try {
                        body->execute(env);
                    } catch (ContinueException&) {
                        env.popFrame();
                        continue;
                    } catch (BreakException&) {
                        env.popFrame();
                        break;
                    }
                    env.popFrame();
                }
                return Value::makeNil();
            }
        };
        auto varname = p->children[0]->value;
        auto iter = buildNode(p->children[1].get());
        auto body = buildNode(p->children[2].get());
        return std::make_unique<F>(varname, std::move(iter), std::move(body));
    }

    ExecNodePtr makeLambda(const ASTNode* p) {
        struct Lambda : ExecNode {
            std::vector<std::string> params;
            ExecNodePtr body;
            Lambda(std::vector<std::string> ps, ExecNodePtr b)
                : params(std::move(ps)), body(std::move(b)) {}
            Value execute(Environment& env) override {
                Value::FuncType fn =
                    [params = this->params, body = this->body.get()](
                        auto const& args, Environment& env) -> Value {
                    if (args.size() != params.size())
                        throw std::runtime_error("Argument count mismatch");
                    env.pushFrame();
                    for (size_t i = 0; i < params.size(); ++i)
                        env.set(params[i], args[i]);
                    try {
                        body->execute(env);
                    } catch (ReturnException& re) {
                        env.popFrame();
                        return re.value;
                    }
                    env.popFrame();
                    return Value::makeNil();
                };
                return Value::makeFunction(std::move(fn));
            }
        };

        std::vector<std::string> ps;
        size_t idx = 0;
        if (p->children[idx]->type == NodeType::ParameterList) {
            for (auto& c : p->children[idx]->children) ps.push_back(c->value);
            ++idx;
        }

        struct Seq : ExecNode {
            std::vector<ExecNodePtr> parts;
            Seq(std::vector<ExecNodePtr> v) : parts(std::move(v)) {}
            Value execute(Environment& env) override {
                for (auto& part : parts) part->execute(env);
                return Value::makeNil();
            }
        };
        std::vector<ExecNodePtr> parts;
        parts.push_back(buildNode(p->children[idx].get()));
        if (idx + 1 < p->children.size() &&
            p->children[idx + 1]->type == NodeType::Return)
            parts.push_back(buildNode(p->children[idx + 1].get()));

        return std::make_unique<Lambda>(
            std::move(ps), std::make_unique<Seq>(std::move(parts)));
    }

    ExecNodePtr makeBinaryOp(const ASTNode* p) {
        struct BO : ExecNode {
            std::string op;
            ExecNodePtr lhs, rhs;
            BO(std::string o, ExecNodePtr l, ExecNodePtr r)
                : op(std::move(o)), lhs(std::move(l)), rhs(std::move(r)) {}
            Value execute(Environment& env) override {
                if (op == "and") {
                    if (!lhs->execute(env).asBoolean())
                        return Value::makeBoolean(false);
                    return Value::makeBoolean(rhs->execute(env).asBoolean());
                }
                if (op == "or") {
                    if (lhs->execute(env).asBoolean())
                        return Value::makeBoolean(true);
                    return Value::makeBoolean(rhs->execute(env).asBoolean());
                }
                auto L = lhs->execute(env);
                auto R = rhs->execute(env);

                if (op == "+") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(L.asNumber() + R.asNumber());
                    type_error("+ supports numbers only");
                }
                if (op == "-") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(L.asNumber() - R.asNumber());
                    type_error("- supports numbers only");
                }
                if (op == "*") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(L.asNumber() * R.asNumber());

                    if (L.type() == Value::Type::String &&
                        R.type() == Value::Type::Number) {
                        std::string out;
                        int times = (int)R.asNumber();
                        while (times-- > 0) out += L.asString();
                        return Value::makeString(out);
                    }
                    type_error("* unsupported types");
                }
                if (op == "/") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(L.asNumber() / R.asNumber());
                    type_error("/ supports numbers only");
                }
                if (op == "^") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(
                            std::pow(L.asNumber(), R.asNumber()));
                    type_error("^ supports numbers only");
                }

                if (op == "==" || op == "!=") {
                    bool eq = L.toString() == R.toString();
                    return Value::makeBoolean(op == "==" ? eq : !eq);
                }
                if ((op == "<" || op == "<=" || op == ">" || op == ">=") &&
                    L.type() == Value::Type::Number &&
                    R.type() == Value::Type::Number) {
                    double a = L.asNumber(), b = R.asNumber();
                    bool res = (op == "<"    ? a < b
                                : op == "<=" ? a <= b
                                : op == ">"  ? a > b
                                             : a >= b);
                    return Value::makeBoolean(res);
                }
                if (op == "index") {
                    if (L.type() != Value::Type::List ||
                        R.type() != Value::Type::Number)
                        type_error("indexing");
                    auto idx = (int)R.asNumber();
                    auto& lst = L.asList();
                    if (idx < 0 || idx >= (int)lst.size())
                        type_error("index out of bounds");
                    return lst[idx];
                }
                type_error("Unknown binary op " + op);
                return Value::makeNil();
            }
        };
        return std::make_unique<BO>(p->value, buildNode(p->children[0].get()),
                                    buildNode(p->children[1].get()));
    }

    ExecNodePtr makeUnaryOp(const ASTNode* p) {
        struct UO : ExecNode {
            std::string op;
            ExecNodePtr arg;
            UO(std::string o, ExecNodePtr a)
                : op(std::move(o)), arg(std::move(a)) {}
            Value execute(Environment& env) override {
                auto v = arg->execute(env);
                if (op == "-") {
                    if (v.type() != Value::Type::Number) type_error("unary -");
                    return Value::makeNumber(-v.asNumber());
                }
                if (op == "not") {
                    return Value::makeBoolean(!v.asBoolean());
                }
                type_error("Unknown unary " + op);
                return Value::makeNil();
            }
        };
        return std::make_unique<UO>(p->value, buildNode(p->children[0].get()));
    }

    ExecNodePtr makeLiteral(const ASTNode* p) {
        struct L : ExecNode {
            Value val;
            L(Value v) : val(std::move(v)) {}
            Value execute(Environment&) override { return val; }
        };
        if (p->type == NodeType::Nil)
            return std::make_unique<L>(Value::makeNil());
        if (p->type == NodeType::Boolean)
            return std::make_unique<L>(Value::makeBoolean(p->value == "true"));

        {
            size_t idx = 0;
            try {
                double d = std::stod(p->value, &idx);
                if (idx == p->value.size()) {
                    return std::make_unique<L>(Value::makeNumber(d));
                }
            } catch (...) {
            }
        }

        return std::make_unique<L>(Value::makeString(p->value));
    }

    ExecNodePtr makeIdentifier(const ASTNode* p) {
        struct ID : ExecNode {
            std::string name;
            ID(std::string n) : name(std::move(n)) {}
            Value execute(Environment& env) override { return env.get(name); }
        };
        return std::make_unique<ID>(p->value);
    }

    ExecNodePtr makeListLiteral(const ASTNode* p) {
        struct LL : ExecNode {
            std::vector<ExecNodePtr> elems;
            Value execute(Environment& env) override {
                std::vector<Value> out;
                out.reserve(elems.size());
                for (auto& e : elems) out.push_back(e->execute(env));
                return Value::makeList(std::move(out));
            }
        };
        auto out = std::make_unique<LL>();
        for (auto& c : p->children) out->elems.push_back(buildNode(c.get()));
        return out;
    }
};

}  // namespace

ExecNodePtr buildExecTree(const ASTNode* ast) { return Builder(ast).build(); }

bool interpret(std::istream& in, std::ostream& out) {
    try {
        std::string src((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

        Lexer lex(src);
        auto tokens = lex.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();

        auto root = buildExecTree(ast.get());

        Environment::Builder eb;
        eb.setOutput(out)
            .addGlobal(
                "print", Value::makeFunction([](auto const& args,
                                                Environment& env) -> Value {
                    for (auto const& v : args) {
                        if (v.type() == Value::Type::String) {
                            const auto& s = v.asString();

                            if (s.find_first_of(" \t\n") != std::string::npos) {
                                env.out() << '"' << s << '"';
                            } else {
                                env.out() << s;
                            }
                        } else {
                            env.out() << v.toString();
                        }
                    }
                    return Value::makeNil();
                }))
            .addGlobal("range", Value::makeFunction([](auto const& args,
                                                       Environment&) -> Value {
                           if (args.size() != 3)
                               throw std::runtime_error("range expects 3 args");
                           int a = (int)args[0].asNumber();
                           int b = (int)args[1].asNumber();
                           int step = (int)args[2].asNumber();
                           if (step == 0)
                               throw std::runtime_error("range step zero");
                           std::vector<Value> out;
                           for (int i = a; (step > 0 ? i < b : i > b);
                                i += step)
                               out.push_back(Value::makeNumber(i));
                           return Value::makeList(std::move(out));
                       }))
            .addGlobal(
                "len", Value::makeFunction([](auto const& args,
                                              Environment&) -> Value {
                    if (args.size() != 1)
                        throw std::runtime_error("len expects 1 arg");
                    if (args[0].type() == Value::Type::String) {
                        return Value::makeNumber(args[0].asString().size());
                    } else if (args[0].type() == Value::Type::List) {
                        return Value::makeNumber(args[0].asList().size());
                    }
                    throw std::runtime_error("len unsupported type");
                }));

        auto env = eb.build();

        root->execute(*env);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace itmoscript
