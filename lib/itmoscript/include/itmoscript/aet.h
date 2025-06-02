
#ifndef ITMOSCRIPT_EXECNODE_H
#define ITMOSCRIPT_EXECNODE_H

#include <memory>

#include "itmoscript/value.h"

namespace itmoscript {

class Environment;
class ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

class AETNode;
using AETNodePtr = std::unique_ptr<AETNode>;

class AETNode {
   public:
    virtual ~AETNode() = default;
    virtual Value execute(Environment& env) = 0;
};

struct ReturnException {
    Value value;
};
struct BreakException {};
struct ContinueException {};

AETNodePtr buildAET(const ASTNode* ast);

}  // namespace itmoscript

#endif
