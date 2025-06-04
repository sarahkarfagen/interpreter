#include <gtest/gtest.h>
#include <itmoscript/interpreter.h>

#include <cmath>

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

// Helper to run code with a given runtime input (for read())
static bool runWithInput(const std::string& code,
                         const std::string& runtimeData, std::string& outStr) {
    std::istringstream codeIn(code);
    std::istringstream runtimeIn(runtimeData);
    std::ostringstream output;
    bool ok = interpret(codeIn, runtimeIn, output);
    if (ok) {
        outStr = output.str();
    } else {
        outStr.clear();
    }
    return ok;
}

TEST(NumberStdLibSuite, AbsPositive) {
    std::string code = R"(
        print(abs(5))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "5");
}

TEST(NumberStdLibSuite, AbsNegative) {
    std::string code = R"(
        print(abs(-3.2))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "3.200000");
}

TEST(NumberStdLibSuite, AbsZero) {
    std::string code = R"(
        print(abs(0))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "0");
}

TEST(NumberStdLibSuite, CeilFraction) {
    std::string code = R"(
        print(ceil(2.1))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "3");
}

TEST(NumberStdLibSuite, CeilInteger) {
    std::string code = R"(
        print(ceil(5.0))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "5");
}

TEST(NumberStdLibSuite, FloorFraction) {
    std::string code = R"(
        print(floor(2.9))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "2");
}

TEST(NumberStdLibSuite, FloorNegative) {
    std::string code = R"(
        print(floor(-2.1))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "-3");
}

TEST(NumberStdLibSuite, RoundHalfDown) {
    std::string code = R"(
        print(round(2.4))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "2");
}

TEST(NumberStdLibSuite, RoundHalfUp) {
    std::string code = R"(
        print(round(2.5))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "3");
}

TEST(NumberStdLibSuite, RoundNegative) {
    std::string code = R"(
        print(round(-2.5))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "-3");
}

TEST(NumberStdLibSuite, SqrtPositive) {
    std::string code = R"(
        print(sqrt(9))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "3");
}

TEST(NumberStdLibSuite, SqrtFraction) {
    std::string code = R"(
        print(sqrt(2))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    double printed = std::stod(out);
    double expected = std::sqrt(2);
    ASSERT_NEAR(printed, expected, 1e-6);
}

TEST(NumberStdLibSuite, SqrtNegativeError) {
    std::string code = R"(
        print(sqrt(-4))
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(NumberStdLibSuite, RndValidRange) {
    std::string code = R"(
        print(rnd(10))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    int val = std::stoi(out);
    ASSERT_GE(val, 0);
    ASSERT_LT(val, 10);
}

TEST(NumberStdLibSuite, RndZeroError) {
    std::string code = R"(
        print(rnd(0))
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(NumberStdLibSuite, ParseNumValidInteger) {
    std::string code = R"(
        print(parse_num("123"))
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(NumberStdLibSuite, ParseNumValidFloat) {
    std::string code = R"(
        print(parse_num("3.14"))
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(NumberStdLibSuite, ParseNumInvalid) {
    std::string code = R"(
        print(parse_num("abc"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "nil");
}

TEST(NumberStdLibSuite, ToStringInteger) {
    std::string code = R"(
        print(to_string(42))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "42");
}

TEST(NumberStdLibSuite, ToStringFloat) {
    std::string code = R"(
        print(to_string(2.718))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "2.718000");
}

TEST(StringStdLibSuite, LenStringEmpty) {
    std::string code = R"(
        print(len(""))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "0");
}

TEST(StringStdLibSuite, LenStringNonEmpty) {
    std::string code = R"(
        print(len("hello"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "5");
}

TEST(StringStdLibSuite, LowercaseAll) {
    std::string code = R"(
        print(lower("HeLLo"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "hello");
}

TEST(StringStdLibSuite, UppercaseAll) {
    std::string code = R"(
        print(upper("HeLLo"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "HELLO");
}

TEST(StringStdLibSuite, SplitByComma) {
    std::string code = R"(
        parts = split("a,b,c", ",")
        print(parts)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[a, b, c]");
}

TEST(StringStdLibSuite, SplitByEmptyDelimiter) {
    std::string code = R"(
        parts = split("abc", "")
        print(parts)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[a, b, c]");
}

TEST(StringStdLibSuite, SplitNoOccurrence) {
    std::string code = R"(
        parts = split("hello", "|")
        print(parts)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[hello]");
}

TEST(StringStdLibSuite, JoinStrings) {
    std::string code = R"(
        lst = ["one", "two", "three"]
        print(join(lst, "-"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "one-two-three");
}

TEST(StringStdLibSuite, JoinEmptyList) {
    std::string code = R"(
        lst = []
        print(join(lst, ","))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "");
}

TEST(StringStdLibSuite, JoinNonStringElementsError) {
    std::string code = R"(
        lst = [1, 2, 3]
        print(join(lst, ","))
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(StringStdLibSuite, ReplaceSingleOccurrence) {
    std::string code = R"(
        print(replace("hello world", "world", "there"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "\"hello there\"");
}

TEST(StringStdLibSuite, ReplaceMultipleOccurrences) {
    std::string code = R"(
        print(replace("ababab", "ab", "cd"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "cdcdcd");
}

TEST(StringStdLibSuite, ReplaceNoOccurrence) {
    std::string code = R"(
        print(replace("hello", "x", "y"))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "hello");
}

TEST(ListStdLibSuite, RangePositiveStep) {
    std::string code = R"(
        lst = range(0, 5, 2)
        print(lst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[0, 2, 4]");
}

TEST(ListStdLibSuite, RangeNegativeStep) {
    std::string code = R"(
        lst = range(5, 0, -2)
        print(lst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[5, 3, 1]");
}

TEST(ListStdLibSuite, RangeStepZeroError) {
    std::string code = R"(
        lst = range(0, 5, 0)
        print(lst)
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(ListStdLibSuite, LenListEmpty) {
    std::string code = R"(
        print(len([]))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "0");
}

TEST(ListStdLibSuite, LenListNonEmpty) {
    std::string code = R"(
        print(len([1,2,3]))
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "3");
}

TEST(ListStdLibSuite, PushElement) {
    std::string code = R"(
        lst = [1,2]
        newlst = push(lst, 3)
        print(newlst)
        print(len(lst))  // original must remain unchanged
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2, 3]2");
}

TEST(ListStdLibSuite, PopElement) {
    std::string code = R"(
        lst = [4,5,6]
        v = pop(lst)
        print(v)
        print(len(lst))  // original list remains unchanged
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "63");
}

TEST(ListStdLibSuite, PopEmptyError) {
    std::string code = R"(
        print(pop([]))
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(ListStdLibSuite, InsertAtBeginning) {
    std::string code = R"(
        lst = [2,3]
        newlst = insert(lst, 0, 1)
        print(newlst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2, 3]");
}

TEST(ListStdLibSuite, InsertInMiddle) {
    std::string code = R"(
        lst = [1,3]
        newlst = insert(lst, 1, 2)
        print(newlst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2, 3]");
}

TEST(ListStdLibSuite, InsertAtEnd) {
    std::string code = R"(
        lst = [1,2]
        newlst = insert(lst, 2, 3)
        print(newlst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2, 3]");
}

TEST(ListStdLibSuite, InsertOutOfBoundsError) {
    std::string code = R"(
        lst = [1,2]
        newlst = insert(lst, 3, 4)
        print(newlst)
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(ListStdLibSuite, RemoveFirstElement) {
    std::string code = R"(
        lst = [1,2,3]
        newlst = remove(lst, 0)
        print(newlst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[2, 3]");
}

TEST(ListStdLibSuite, RemoveMiddleElement) {
    std::string code = R"(
        lst = [1,2,3,4]
        newlst = remove(lst, 2)
        print(newlst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2, 4]");
}

TEST(ListStdLibSuite, RemoveLastElement) {
    std::string code = R"(
        lst = [1,2,3]
        newlst = remove(lst, 2)
        print(newlst)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2]");
}

TEST(ListStdLibSuite, RemoveOutOfBoundsError) {
    std::string code = R"(
        lst = [1,2]
        newlst = remove(lst, 2)
        print(newlst)
    )";
    std::string out;
    ASSERT_FALSE(run(code, out));
}

TEST(ListStdLibSuite, SortNumbersAscending) {
    std::string code = R"(
        lst = [3,1,2]
        sorted = sort(lst)
        print(sorted)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 2, 3]");
}

TEST(ListStdLibSuite, SortStrings) {
    std::string code = R"(
        lst = ["b", "a", "c"]
        sorted = sort(lst)
        print(sorted)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[a, b, c]");
}

TEST(ListStdLibSuite, SortMixedTypes) {
    std::string code = R"(
        lst = ["2", 1, "10"]
        sorted = sort(lst)
        print(sorted)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[1, 10, 2]");
}

TEST(ListStdLibSuite, CombinedOperations) {
    std::string code = R"(
        lst = range(1, 6, 1)      // [1,2,3,4,5]
        lst2 = remove(lst, 0)     // [2,3,4,5]
        lst3 = push(lst2, 10)     // [2,3,4,5,10]
        lst4 = insert(lst3, 2, 7) // [2,3,7,4,5,10]
        sorted = sort(lst4, function(a, b) return a < b end function)       // [2,3,4,5,7,10]
        print(sorted)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[2, 3, 4, 5, 7, 10]");
}

TEST(SystemStdLibSuite, PrintNoNewline) {
    std::string code = R"(
        print("hello")
        print("world")
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    // Should be "helloworld", no line breaks
    ASSERT_EQ(out, "helloworld");
}

TEST(SystemStdLibSuite, PrintNumberAndString) {
    std::string code = R"(
        print(123)
        print("abc")
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "123abc");
}

TEST(SystemStdLibSuite, PrintlnAddsNewline) {
    std::string code = R"(
        println("line1")
        println("line2")
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    // Each println adds exactly one '\n'
    ASSERT_EQ(out, "line1\nline2\n");
}

TEST(SystemStdLibSuite, MixedPrintAndPrintln) {
    std::string code = R"(
        print("a")
        println("b")
        print("c")
        println("d")
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    // "a" then "b\n", then "c" then "d\n"
    ASSERT_EQ(out, "ab\ncd\n");
}

TEST(SystemStdLibSuite, ReadSingleLine) {
    std::string code = R"(
        x = read()
        print(x)
    )";
    std::string runtime = "hello\n";
    std::string out;
    ASSERT_TRUE(runWithInput(code, runtime, out));
    // read() returns "hello" (no '\n'), then print prints it without quotes if
    // no spaces? Actually, since "hello" has no whitespace, print prints it
    // raw:
    ASSERT_EQ(out, "hello");
}

TEST(SystemStdLibSuite, ReadThenPrintQuoted) {
    std::string code = R"(
        x = read()
        print(x)
    )";
    std::string runtime = "hello world\n";
    std::string out;
    ASSERT_TRUE(runWithInput(code, runtime, out));
    // read() returns "hello world"; print sees a space, prints as "hello world"
    ASSERT_EQ(out, "\"hello world\"");
}

TEST(SystemStdLibSuite, ReadEOFReturnsNil) {
    std::string code = R"(
        x = read()
        print(x)
    )";
    std::string runtime = "";  // empty input
    std::string out;
    ASSERT_TRUE(runWithInput(code, runtime, out));
    // read() returns nil, printed as "nil"
    ASSERT_EQ(out, "nil");
}

TEST(SystemStdLibSuite, StacktraceEmptyOutsideFunction) {
    std::string code = R"(
        st = stacktrace()
        print(st)
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    // No functions on the stack, so we get an empty list "[]"
    ASSERT_EQ(out, "[]");
}

TEST(SystemStdLibSuite, StacktraceOneLevel) {
    std::string code = R"(
        single = function()
            st = stacktrace()
            print(st)
        end function

        single()
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[<anonymous>]");
}

TEST(SystemStdLibSuite, StacktraceNested) {
    std::string code = R"(
        inner = function()
            st = stacktrace()
            print(st)
        end function

        outer = function()
            inner()
        end function

        outer()
    )";
    std::string out;
    ASSERT_TRUE(run(code, out));
    ASSERT_EQ(out, "[<anonymous>, <anonymous>]");
}
