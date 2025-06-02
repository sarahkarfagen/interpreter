#include <gtest/gtest.h>
#include <itmoscript/interpreter.h>

#include <string>
#include <vector>

using namespace itmoscript;

TEST(BasicSuite, MultiLineStatement) {
    std::string code = R"(
        mass = s = 1
        print(mass)
        print(s)
    )";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_FALSE(interpret(input, output));
    ASSERT_EQ(output.str(), "1\n1");
}
