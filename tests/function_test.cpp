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

#include <gtest/gtest.h>
#include <itmoscript/interpreter.h>

using namespace itmoscript;

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

    std::string expected = "1020nil";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

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

TEST(FunctionTestSuite, NoReturnImpliesNil) {
    std::string code = R"(
        side = function(x)
            y = x + 5    // no return 
        end function

        r = side(2)
        print(r)
    )";

    std::string expected = "nil";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

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

    std::string expected = "truefalse";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

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

TEST(FunctionTestSuite, BuiltinTooFewArgsError) {
    std::string code = R"(
        print(len())
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

TEST(FunctionTestSuite, BuiltinTooManyArgsError) {
    std::string code = R"(
        print(len("hi", "extra"))
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

TEST(FunctionTestSuite, BuiltinAsListElement) {
    std::string code = R"(
        funcs = [len, len, len]
        
        print( funcs[0]("abc") )
        print( funcs[1]("d") )
        print( funcs[2]("xyz") )
    )";

    std::string expected = "313";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

TEST(FunctionTestSuite, AnonymousFunctionStacktrace) {
    std::string code = R"(
        
        (function(x)
            st = stacktrace()
            print(len(st))  
        end function)(5)
    )";

    std::string expected = "1";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

TEST(FunctionTestSuite, ShadowingNotClosure) {
    std::string code = R"(
        outer = function(x)
            
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

    std::string expected = "noyes";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

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

TEST(FunctionTestSuite, FibonacciRecursion) {
    std::string code = R"(
        fib = function(n)
            if n == 0 then return 0 end if
            if n == 1 then return 1 end if
            return fib(n - 1) + fib(n - 2)
        end function

        print(fib(6))
    )";

    std::string expected = "8";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, expected);
}

TEST(FunctionTestSuite, CallingNonFunctionError) {
    std::string code = R"(
        x = 5
        print(x(2))
    )";

    std::string out;
    ASSERT_FALSE(run(code, out));
    ASSERT_TRUE(out.empty());
}

TEST(FunctionTestSuite, ShadowBuiltinFunction) {
    std::string code = R"(
        len = function(x) return x * 2 end function
        print(len(5))      // should use user function → 10
        print(len(nil))    // error inside user function, as nil*2 invalid
    )";

    std::string out;

    ASSERT_FALSE(run(code, out));
}
