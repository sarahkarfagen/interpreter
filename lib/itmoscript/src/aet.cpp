
#include "itmoscript/aet.h"

#include <cmath>
#include <stdexcept>

#include "itmoscript/ast.h"
#include "itmoscript/environment.h"

namespace itmoscript {

namespace {

inline void type_error(const std::string& what) {
    throw std::runtime_error("Type error: " + what);
}

class Builder {
    const ASTNode* ast_;

   public:
    explicit Builder(const ASTNode* root) : ast_(root) {}
    AETNodePtr build() { return buildNode(ast_); }

   private:
    AETNodePtr buildNode(const ASTNode* node) {
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

    AETNodePtr makeProgram(const ASTNode* p) {
        return buildNode(p->children[0].get());
    }

    AETNodePtr makeStmtList(const ASTNode* p) {
        struct SL : AETNode {
            std::vector<AETNodePtr> stmts;
            Value execute(Environment& env) override {
                for (auto& s : stmts) {
                    s->execute(env);
                }
                return Value::makeNil();
            }
        };
        auto out = std::make_unique<SL>();
        for (auto& c : p->children) {
            out->stmts.push_back(buildNode(c.get()));
        }
        return out;
    }

    AETNodePtr makeAssignment(const ASTNode* p) {
        struct A : AETNode {
            std::string name, op;
            AETNodePtr expr;
            A(std::string n, std::string o, AETNodePtr e)
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
                env.set(name, std::move(v));
                return Value::makeNil();
            }
        };

        auto var = p->value;
        auto op = p->children[1]->value;
        auto rhs = buildNode(p->children[2].get());
        return std::make_unique<A>(var, op, std::move(rhs));
    }

    AETNodePtr makeFuncCall(const ASTNode* p) {
        struct FC : AETNode {
            AETNodePtr expr;
            std::vector<AETNodePtr> args;
            FC(AETNodePtr e, std::vector<AETNodePtr> a)
                : expr(std::move(e)), args(std::move(a)) {}
            Value execute(Environment& env) override {
                auto fval = expr->execute(env);
                if (fval.type() != Value::Type::Function)
                    type_error("Not a function: " + fval.toString());
                std::vector<Value> avals;
                avals.reserve(args.size());
                for (auto& a : args) {
                    avals.push_back(a->execute(env));
                }
                return fval.asFunction()(avals, env);
            }
        };
        std::vector<AETNodePtr> args;
        if (p->children.size() > 1) {
            for (auto& c0 : p->children[1]->children) {
                args.push_back(buildNode(c0.get()));
            }
        }
        return std::make_unique<FC>(buildNode(p->children[0].get()),
                                    std::move(args));
    }

    AETNodePtr makeReturn(const ASTNode* p) {
        struct R : AETNode {
            AETNodePtr expr;
            R(AETNodePtr e) : expr(std::move(e)) {}
            Value execute(Environment& env) override {
                auto v = expr->execute(env);
                throw ReturnException{v};
            }
        };
        return std::make_unique<R>(buildNode(p->children[0].get()));
    }

    AETNodePtr makeBreak() {
        struct B : AETNode {
            Value execute(Environment&) override { throw BreakException{}; }
        };
        return std::make_unique<B>();
    }

    AETNodePtr makeContinue() {
        struct C : AETNode {
            Value execute(Environment&) override { throw ContinueException{}; }
        };
        return std::make_unique<C>();
    }

    AETNodePtr makeIf(const ASTNode* p) {
        struct I : AETNode {
            std::vector<std::pair<AETNodePtr, AETNodePtr>> clauses;
            AETNodePtr elseBody;
            Value execute(Environment& env) override {
                for (auto& [cond, body] : clauses) {
                    if (cond->execute(env).asBoolean()) {
                        return body->execute(env);
                    }
                }
                if (elseBody) {
                    return elseBody->execute(env);
                }
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

    AETNodePtr makeWhile(const ASTNode* p) {
        struct W : AETNode {
            AETNodePtr cond, body;
            W(AETNodePtr c, AETNodePtr b)
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

    AETNodePtr makeFor(const ASTNode* p) {
        struct F : AETNode {
            std::string var;
            AETNodePtr iterable, body;
            F(std::string v, AETNodePtr it, AETNodePtr b)
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

    AETNodePtr makeLambda(const ASTNode* p) {
        struct Lambda : AETNode {
            std::vector<std::string> params;
            AETNodePtr body;
            Lambda(std::vector<std::string> ps, AETNodePtr b)
                : params(std::move(ps)), body(std::move(b)) {}
            Value execute(Environment& env) override {
                Value::FuncType fn = [params = this->params,
                                      body = this->body.get()](
                                         const std::vector<Value>& args,
                                         Environment& env) -> Value {
                    if (args.size() != params.size())
                        throw std::runtime_error("Argument count mismatch");
                    env.pushFrame();
                    for (size_t i = 0; i < params.size(); ++i) {
                        env.set(params[i], args[i]);
                    }
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
            for (auto& c : p->children[idx]->children) {
                ps.push_back(c->value);
            }
            ++idx;
        }

        struct Seq : AETNode {
            std::vector<AETNodePtr> parts;
            Seq(std::vector<AETNodePtr> v) : parts(std::move(v)) {}
            Value execute(Environment& env) override {
                for (auto& part : parts) {
                    part->execute(env);
                }
                return Value::makeNil();
            }
        };

        std::vector<AETNodePtr> parts;
        parts.push_back(buildNode(p->children[idx].get()));
        if (idx + 1 < p->children.size() &&
            p->children[idx + 1]->type == NodeType::Return) {
            parts.push_back(buildNode(p->children[idx + 1].get()));
        }

        return std::make_unique<Lambda>(
            std::move(ps), std::make_unique<Seq>(std::move(parts)));
    }

    AETNodePtr makeBinaryOp(const ASTNode* p) {
        struct BO : AETNode {
            std::string op;
            AETNodePtr lhs, rhs;
            BO(std::string o, AETNodePtr l, AETNodePtr r)
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
                        int times = static_cast<int>(R.asNumber());
                        while (times-- > 0) {
                            out += L.asString();
                        }
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
                        R.type() == Value::Type::Number) {
                        return Value::makeNumber(
                            std::pow(L.asNumber(), R.asNumber()));
                    }
                    type_error("^ supports numbers only");
                }
                if (op == "==" || op == "!=") {
                    bool eq = (L.toString() == R.toString());
                    return Value::makeBoolean(op == "==" ? eq : !eq);
                }
                if ((op == "<" || op == "<=" || op == ">" || op == ">=") &&
                    L.type() == Value::Type::Number &&
                    R.type() == Value::Type::Number) {
                    double a = L.asNumber();
                    double b = R.asNumber();
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
                    int idx = static_cast<int>(R.asNumber());
                    const auto& lst = L.asList();
                    if (idx < 0 || idx >= static_cast<int>(lst.size()))
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

    AETNodePtr makeUnaryOp(const ASTNode* p) {
        struct UO : AETNode {
            std::string op;
            AETNodePtr arg;
            UO(std::string o, AETNodePtr a)
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

    AETNodePtr makeLiteral(const ASTNode* p) {
        struct L : AETNode {
            Value val;
            L(Value v) : val(std::move(v)) {}
            Value execute(Environment&) override { return val; }
        };

        if (p->type == NodeType::Nil) {
            return std::make_unique<L>(Value::makeNil());
        }
        if (p->type == NodeType::Boolean) {
            return std::make_unique<L>(Value::makeBoolean(p->value == "true"));
        }

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

    AETNodePtr makeIdentifier(const ASTNode* p) {
        struct ID : AETNode {
            std::string name;
            ID(std::string n) : name(std::move(n)) {}
            Value execute(Environment& env) override { return env.get(name); }
        };
        return std::make_unique<ID>(p->value);
    }

    AETNodePtr makeListLiteral(const ASTNode* p) {
        struct LL : AETNode {
            std::vector<AETNodePtr> elems;
            Value execute(Environment& env) override {
                std::vector<Value> out;
                out.reserve(elems.size());
                for (auto& e : elems) {
                    out.push_back(e->execute(env));
                }
                return Value::makeList(std::move(out));
            }
        };
        auto out = std::make_unique<LL>();
        for (auto& c : p->children) {
            out->elems.push_back(buildNode(c.get()));
        }
        return out;
    }
};

}  // namespace

AETNodePtr buildAET(const ASTNode* ast) { return Builder(ast).build(); }

}  // namespace itmoscript
