

#include "itmoscript/aet.h"

#include <cmath>
#include <stdexcept>
#include <unordered_map>

#include "itmoscript/ast.h"
#include "itmoscript/environment.h"

namespace itmoscript {

namespace {

inline void type_error(const std::string& what) {
    throw std::runtime_error("Type error: " + what);
}

bool isTruthy(const Value& v) {
    switch (v.type()) {
        case Value::Type::Boolean:
            return v.asBoolean();
        case Value::Type::Nil:
            return false;
        case Value::Type::Number:
            return v.asNumber() != 0;
        default:
            return true;
    }
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

                    if (op == "+=") {
                        if (old.type() == Value::Type::Number &&
                            v.type() == Value::Type::Number) {
                            v = Value::makeNumber(old.asNumber() +
                                                  v.asNumber());
                        }

                        else if (old.type() == Value::Type::String &&
                                 v.type() == Value::Type::String) {
                            v = Value::makeString(old.asString() +
                                                  v.asString());
                        }

                        else if (old.type() == Value::Type::List &&
                                 v.type() == Value::Type::List) {
                            auto out = old.asList();
                            const auto& rhs = v.asList();
                            out.insert(out.end(), rhs.begin(), rhs.end());
                            v = Value::makeList(std::move(out));
                        } else {
                            type_error("'+=' unsupported types");
                        }
                    }

                    else if (op == "-=") {
                        if (old.type() == Value::Type::Number &&
                            v.type() == Value::Type::Number) {
                            v = Value::makeNumber(old.asNumber() -
                                                  v.asNumber());
                        }

                        else if (old.type() == Value::Type::String &&
                                 v.type() == Value::Type::String) {
                            const auto& a = old.asString();
                            const auto& b = v.asString();
                            if (a.size() >= b.size() &&
                                a.compare(a.size() - b.size(), b.size(), b) ==
                                    0) {
                                v = Value::makeString(
                                    a.substr(0, a.size() - b.size()));
                            } else {
                                type_error("'-=' suffix not found");
                            }
                        } else {
                            type_error("'-=' unsupported types");
                        }
                    }

                    else if (op == "*=") {
                        if (old.type() == Value::Type::Number &&
                            v.type() == Value::Type::Number) {
                            v = Value::makeNumber(old.asNumber() *
                                                  v.asNumber());
                        }

                        else if (old.type() == Value::Type::String &&
                                 v.type() == Value::Type::Number) {
                            std::string out;
                            int times = static_cast<int>(v.asNumber());
                            while (times-- > 0) out += old.asString();
                            v = Value::makeString(std::move(out));
                        }

                        else if (old.type() == Value::Type::List &&
                                 v.type() == Value::Type::Number) {
                            auto base = old.asList();
                            std::vector<Value> out;
                            int times = static_cast<int>(v.asNumber());
                            while (times-- > 0)
                                out.insert(out.end(), base.begin(), base.end());
                            v = Value::makeList(std::move(out));
                        } else {
                            type_error("'*=' unsupported types");
                        }
                    }

                    else {
                        if (old.type() != Value::Type::Number ||
                            v.type() != Value::Type::Number) {
                            type_error(op + " requires numbers");
                        }
                        double a = old.asNumber(), b = v.asNumber();
                        if (op == "/=")
                            v = Value::makeNumber(a / b);
                        else if (op == "%=")
                            v = Value::makeNumber(std::fmod(a, b));
                        else if (op == "^=")
                            v = Value::makeNumber(std::pow(a, b));
                        else
                            type_error("Unsupported op '" + op + "'");
                    }
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
                    Value cv = cond->execute(env);
                    if (isTruthy(cv)) {
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
                while (isTruthy(cond->execute(env))) {
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
        std::string fnName = p->value;

        std::vector<std::string> params;
        size_t idx = 0;
        if (!p->children.empty() &&
            p->children[idx]->type == NodeType::ParameterList) {
            for (auto& c : p->children[idx]->children) {
                params.push_back(c->value);
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

        struct LambdaNode : AETNode {
            std::string name;
            std::vector<std::string> params;
            AETNodePtr body;

            LambdaNode(std::string n, std::vector<std::string> ps, AETNodePtr b)
                : name(std::move(n)),
                  params(std::move(ps)),
                  body(std::move(b)) {}

            Value execute(Environment& env) override {
                auto const& locals = env.getLocals();
                std::unordered_map<std::string, Value> capturedClosure = locals;

                auto capturedName = name;
                auto capturedParams = params;
                AETNode* bodyPtr = body.get();

                Value::FuncType fn =
                    [capturedName, capturedParams, bodyPtr, capturedClosure](
                        auto const& args, Environment& env2) -> Value {
                    env2.pushStack(capturedName.empty() ? "<anonymous>"
                                                        : capturedName);

                    env2.pushFrame();

                    for (auto const& kv : capturedClosure) {
                        env2.set(kv.first, kv.second);
                    }

                    if (args.size() > capturedParams.size()) {
                        env2.popFrame();
                        env2.popStack();
                        throw std::runtime_error(
                            "Argument count mismatch in function '" +
                            capturedName + "' (expected at most " +
                            std::to_string(capturedParams.size()) + ", got " +
                            std::to_string(args.size()) + ")");
                    }
                    for (size_t i = 0; i < args.size(); ++i) {
                        env2.set(capturedParams[i], args[i]);
                    }
                    for (size_t i = args.size(); i < capturedParams.size();
                         ++i) {
                        env2.set(capturedParams[i], Value::makeNil());
                    }

                    try {
                        bodyPtr->execute(env2);
                    } catch (ReturnException& re) {
                        Value ret = re.value;
                        env2.popFrame();
                        env2.popStack();
                        return ret;
                    }

                    env2.popFrame();
                    env2.popStack();
                    return Value::makeNil();
                };

                return Value::makeFunction(std::move(fn));
            }
        };

        return std::make_unique<LambdaNode>(
            fnName, std::move(params), std::make_unique<Seq>(std::move(parts)));
    }

    AETNodePtr makeBinaryOp(const ASTNode* p) {
        struct BO : AETNode {
            std::string op;
            AETNodePtr lhs, rhs;
            BO(std::string o, AETNodePtr l, AETNodePtr r)
                : op(std::move(o)), lhs(std::move(l)), rhs(std::move(r)) {}
            Value execute(Environment& env) override {
                if (op == ":") {
                    Value startVal = lhs->execute(env);
                    Value endVal = rhs->execute(env);
                    std::vector<Value> spec;
                    spec.reserve(2);
                    spec.push_back(std::move(startVal));
                    spec.push_back(std::move(endVal));
                    return Value::makeList(std::move(spec));
                }

                if (op == "and") {
                    if (!isTruthy(lhs->execute(env)))
                        return Value::makeBoolean(false);
                    return Value::makeBoolean(isTruthy(rhs->execute(env)));
                }
                if (op == "or") {
                    if (isTruthy(lhs->execute(env)))
                        return Value::makeBoolean(true);
                    return Value::makeBoolean(isTruthy(rhs->execute(env)));
                }

                auto L = lhs->execute(env);
                auto R = rhs->execute(env);

                if (op == "+") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number) {
                        return Value::makeNumber(L.asNumber() + R.asNumber());
                    }

                    if (L.type() == Value::Type::String &&
                        R.type() == Value::Type::String) {
                        return Value::makeString(L.asString() + R.asString());
                    }

                    if (L.type() == Value::Type::List &&
                        R.type() == Value::Type::List) {
                        auto out = L.asList();
                        const auto& rhs = R.asList();
                        out.insert(out.end(), rhs.begin(), rhs.end());
                        return Value::makeList(std::move(out));
                    }
                    type_error("+ unsupported types");
                }
                if (op == "-") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number) {
                        return Value::makeNumber(L.asNumber() - R.asNumber());
                    }

                    if (L.type() == Value::Type::String &&
                        R.type() == Value::Type::String) {
                        const auto& a = L.asString();
                        const auto& b = R.asString();
                        if (a.size() >= b.size() &&
                            a.compare(a.size() - b.size(), b.size(), b) == 0) {
                            return Value::makeString(
                                a.substr(0, a.size() - b.size()));
                        }
                        type_error("- string suffix not found");
                    }
                    type_error("- unsupported types");
                }
                if (op == "*") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number) {
                        return Value::makeNumber(L.asNumber() * R.asNumber());
                    }

                    if (L.type() == Value::Type::String &&
                        R.type() == Value::Type::Number) {
                        std::string out;
                        int times = static_cast<int>(R.asNumber());
                        while (times-- > 0) {
                            out += L.asString();
                        }
                        return Value::makeString(out);
                    }

                    if (L.type() == Value::Type::List &&
                        R.type() == Value::Type::Number) {
                        auto base = L.asList();
                        std::vector<Value> out;
                        int times = static_cast<int>(R.asNumber());
                        while (times-- > 0) {
                            out.insert(out.end(), base.begin(), base.end());
                        }
                        return Value::makeList(std::move(out));
                    }
                    type_error("* unsupported types");
                }
                if (op == "/") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(L.asNumber() / R.asNumber());
                    type_error("/ supports numbers only");
                }
                if (op == "%") {
                    if (L.type() == Value::Type::Number &&
                        R.type() == Value::Type::Number)
                        return Value::makeNumber(
                            std::fmod(L.asNumber(), R.asNumber()));
                    type_error("% supports numbers only");
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
                    if (L.type() == Value::Type::List) {
                        const auto& lst = L.asList();
                        int n = static_cast<int>(lst.size());

                        if (R.type() == Value::Type::Number) {
                            int i = static_cast<int>(R.asNumber());
                            if (i < 0) i += n;
                            if (i < 0 || i >= n)
                                type_error("index out of bounds");
                            return lst[i];
                        }

                        if (R.type() == Value::Type::List) {
                            const auto& sp = R.asList();
                            if (sp.size() != 2)
                                type_error("slice spec must have 2 elements");

                            int start = 0, end = n;
                            if (sp[0].type() == Value::Type::Number) {
                                start = static_cast<int>(sp[0].asNumber());
                                if (start < 0) start += n;
                            }
                            if (sp[1].type() == Value::Type::Number) {
                                end = static_cast<int>(sp[1].asNumber());
                                if (end < 0) end += n;
                            }

                            start = std::clamp(start, 0, n);
                            end = std::clamp(end, 0, n);
                            std::vector<Value> out;
                            for (int i = start; i < end; ++i)
                                out.push_back(lst[i]);
                            return Value::makeList(std::move(out));
                        }
                    }

                    if (L.type() == Value::Type::String) {
                        const auto& s = L.asString();
                        int n = static_cast<int>(s.size());
                        if (R.type() == Value::Type::Number) {
                            int i = static_cast<int>(R.asNumber());
                            if (i < 0) i += n;
                            if (i < 0 || i >= n)
                                type_error("index out of bounds");
                            return Value::makeString(std::string(1, s[i]));
                        }
                        if (R.type() == Value::Type::List) {
                            const auto& sp = R.asList();
                            if (sp.size() != 2)
                                type_error("slice spec must have 2 elements");
                            int start = 0, end = n;
                            if (sp[0].type() == Value::Type::Number) {
                                start = static_cast<int>(sp[0].asNumber());
                                if (start < 0) start += n;
                            }
                            if (sp[1].type() == Value::Type::Number) {
                                end = static_cast<int>(sp[1].asNumber());
                                if (end < 0) end += n;
                            }
                            start = std::clamp(start, 0, n);
                            end = std::clamp(end, 0, n);
                            if (end <= start) return Value::makeString("");
                            return Value::makeString(
                                s.substr(start, end - start));
                        }
                    }
                    type_error("indexing/slicing requires list or string");
                }
                type_error("Unknown binary op " + op);
                return Value::makeNil();
            }
        };

        AETNodePtr left = buildNode(p->children[0].get());
        AETNodePtr right;
        if (p->value == ":" && p->children.size() < 2) {
            ASTNode tmpNil(NodeType::Nil);
            right = buildNode(&tmpNil);
        } else {
            right = buildNode(p->children[1].get());
        }
        return std::make_unique<BO>(p->value, std::move(left),
                                    std::move(right));
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
                    return Value::makeBoolean(!isTruthy(v));
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
