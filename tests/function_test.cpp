#include <gtest/gtest.h>
#include <itmoscript/interpreter.h>

using namespace itmoscript;

TEST(FunctionTestSuite, SimpleFunctionTest) {
    std::string code = R"(
        incr = function(value)
            return value + 1
        end function

        x = incr(2)
        print(x)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}

TEST(FunctionTestSuite, FunctionAsArgTest) {
    std::string code = R"(
        incr = function(value)
            return value + 1
        end function

        printresult = function(value, func)
            result = func(value)
            print(result)
        end function

        printresult(2, incr)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}

TEST(FunctionTestSuite, NestedFunctionTest) {
    std::string code = R"(
        // NB: inner and outer `value` are different symbols.
        // You are not required to implement closures (aka lambdas).

        incr_and_print = function(value)
            incr = function(value)
                return value + 1
            end function

            print(incr(value))
        end function

        incr_and_print(2)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}

TEST(FunctionTestSuite, FunnySyntaxTest) {
    std::string code = R"(
        funcs = [
            function() return 1 end function,
            function() return 2 end function,
            function() return 3 end function,
        ]

        print(funcs[0]())
        print(funcs[1]())
        print(funcs[2]())
    )";

    std::string expected = "123";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}

// tests/function_test.cpp

#include <gtest/gtest.h>
#include <itmoscript/interpreter.h>

using namespace itmoscript;

// Helper to run code with no runtime input
static bool run(const std::string& code, std::string& outStr) {
    std::istringstream input(code);
    std::ostringstream output;
    bool ok = interpret(input, output);
    if (ok) {
        outStr = output.str();
    } else {
        outStr.clear();
    }
    return ok;
}

// 1. Missing arguments should be filled with nil
TEST(FunctionTestSuite, MissingArgumentsBecomeNil) {
    std::string code = R"(
        f = function(a, b, c)
            print(a)
            print(b)
            print(c)
        end function

        // Only pass 2 args: third should be nil
        f(10, 20)
    )";

    // print(10) → "10"
    // print(20) → "20"
    // print(nil) → "nil"
    std::string expected = "1020nil";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 2. Too many arguments should error
TEST(FunctionTestSuite, TooManyArgumentsError) {
    std::string code = R"(
        f = function(a, b)
            return a + b
        end function

        print(f(1, 2, 3))
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

// 3. No‐argument function invoked with zero args
TEST(FunctionTestSuite, NoArgFunctionWorks) {
    std::string code = R"(
        zero = function()
            return 0
        end function

        print(zero())
    )";

    std::string expected = "0";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 4. No‐argument function invoked with extra args → error
TEST(FunctionTestSuite, NoArgFunctionExtraArgsError) {
    std::string code = R"(
        zero = function()
            return 0
        end function

        print(zero(1))
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

// 5. Function with internal side‐effect and no explicit return: returns nil
TEST(FunctionTestSuite, NoReturnImpliesNil) {
    std::string code = R"(
        side = function(x)
            y = x + 5    // no return
        end function

        r = side(2)
        print(r)
    )";

    // side(2) returns nil; print(nil) → "nil"
    std::string expected = "nil";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 6. Recursive function (factorial)
TEST(FunctionTestSuite, RecursiveFunctionFactorial) {
    std::string code = R"(
        fact = function(n)
            if n == 0 then
                return 1
            end if
            return n * fact(n - 1)
        end function

        print(fact(5))
    )";

    std::string expected = "120";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 7. Mutual recursion (even/odd)
TEST(FunctionTestSuite, MutualRecursionEvenOdd) {
    std::string code = R"(
        is_even = function(n)
            if n == 0 then
                return true
            end if
            return is_odd(n - 1)
        end function

        is_odd = function(n)
            if n == 0 then
                return false
            end if
            return is_even(n - 1)
        end function

        print(is_even(10))
        print(is_odd(10))
    )";

    // true → "true", false → "false"
    std::string expected = "truefalse";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 8. Higher‐order function returning a function
TEST(FunctionTestSuite, FunctionReturningFunction) {
    std::string code = R"(
        makeAdder = function(x)
            return function(y) return x + y end function
        end function

        add5 = makeAdder(5)
        print(add5(3))
    )";

    std::string expected = "8";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 9. Passing lambda directly as argument and invoking it
TEST(FunctionTestSuite, LambdaAsInlineArgument) {
    std::string code = R"(
        apply = function(f, x)
            return f(x)
        end function

        print( apply(function(z) return z * 2 end function, 7) )
    )";

    std::string expected = "14";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 10. Functions stored in a list and invoked
TEST(FunctionTestSuite, FunctionsInListAndInvoke) {
    std::string code = R"(
        f1 = function() return 1 end function
        f2 = function() return 2 end function
        f3 = function() return 3 end function

        fl = [f1, f2, f3]
        print(fl[0]())
        print(fl[1]())
        print(fl[2]())
    )";

    std::string expected = "123";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 11. Passing too few args to built‐in functions (e.g., len) → error
TEST(FunctionTestSuite, BuiltinTooFewArgsError) {
    std::string code = R"(
        print(len())
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

// 12. Passing too many args to built‐in functions (e.g., len) → error
TEST(FunctionTestSuite, BuiltinTooManyArgsError) {
    std::string code = R"(
        print(len("hi", "extra"))
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

// 13. Using a built‐in function as a value inside a list
TEST(FunctionTestSuite, BuiltinAsListElement) {
    std::string code = R"(
        funcs = [len, len, len]
        // All elements are the same built‐in reference
        print( funcs[0]("abc") )
        print( funcs[1]("d") )
        print( funcs[2]("xyz") )
    )";

    // len("abc")=3, len("d")=1, len("xyz")=3
    std::string expected = "313";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 14. Anonymous functions (no name), check call‐stack reflection
TEST(FunctionTestSuite, AnonymousFunctionStacktrace) {
    std::string code = R"(
        // Define and immediately call an anonymous function
        (function(x)
            st = stacktrace()
            print(len(st))  // stack depth should be 1 (the anonymous function itself)
        end function)(5)
    )";

    std::string expected = "1";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 15. Function returns another function that uses outer parameter (no true
// closure; tests shadowing)
TEST(FunctionTestSuite, ShadowingNotClosure) {
    std::string code = R"(
        outer = function(x)
            // inner captures x by name, not by reference; but since each call rebuilds, it's okay
            inner = function() return x end function
            return inner
        end function

        f1 = outer(10)
        print(f1())
        f2 = outer(20)
        print(f2())
    )";

    std::string expected = "1020";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 16. Verify that a function parameter defaulting to nil can be tested
TEST(FunctionTestSuite, ParameterNilInCondition) {
    std::string code = R"(
        test = function(a)
            if a then
                print("yes")
            else
                print("no")
            end if
        end function

        test()       // a=nil → treated as false → "no"
        test(true)   // a=true → "yes"
    )";

    std::string expected = "\"no\"\"yes\"";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 17. Return multiple statements (only first return counts)
TEST(FunctionTestSuite, FirstReturnOnly) {
    std::string code = R"(
        f = function(x)
            if x < 0 then
                return -1
                return 0    // unreachable
            end if
            return x
            return 0        // unreachable
        end function

        print(f(-5))
        print(f(3))
    )";

    std::string expected = "-13";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 18. Deep recursion to test stack behavior (e.g., fibonacci)
TEST(FunctionTestSuite, FibonacciRecursion) {
    std::string code = R"(
        fib = function(n)
            if n == 0 then return 0 end if
            if n == 1 then return 1 end if
            return fib(n - 1) + fib(n - 2)
        end function

        print(fib(6))
    )";

    // fib(6)=8
    std::string expected = "8";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

// 19. Error when calling a non‐function value
TEST(FunctionTestSuite, CallingNonFunctionError) {
    std::string code = R"(
        x = 5
        print(x(2))
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

// 20. Function shadowing built‐in: local function named "len"
TEST(FunctionTestSuite, ShadowBuiltinFunction) {
    std::string code = R"(
        len = function(x) return x * 2 end function
        print(len(5))      // should use user function → 10
        print(len("a"))    // error inside user function, as "a"*2 invalid
    )";

    std::string out;
    // First call prints "10"; second call errors because "a" is string.
    ASSERT_FALSE(run(code, out));
    // However, the side‐effect from first print may appear before error occurs:
    // So out may equal "10" or "10" without newline.
    ASSERT_TRUE(out == "10" || out == "10");
}
