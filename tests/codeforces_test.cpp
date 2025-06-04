#include <gtest/gtest.h>
#include <itmoscript/interpreter.h>

#include <fstream>
#include <sstream>
#include <string>

#ifndef PROJECT_ROOT_DIR
#error "PROJECT_ROOT_DIR must be defined by CMake for this test-suite to run"
#endif

using namespace itmoscript;

static void runCodeforcesTest(const std::string& solutionRel,
                              const std::string& inputRel,
                              const std::string& expectedRel) {
    std::string solutionPath =
        std::string(PROJECT_ROOT_DIR) + "/" + solutionRel;
    std::string inputPath = std::string(PROJECT_ROOT_DIR) + "/" + inputRel;
    std::string expectedPath =
        std::string(PROJECT_ROOT_DIR) + "/" + expectedRel;

    std::ifstream solFile(solutionPath);
    ASSERT_TRUE(solFile.is_open())
        << "Failed to open solution file: " << solutionPath;
    std::ostringstream solBuf;
    solBuf << solFile.rdbuf();
    std::string code = solBuf.str();

    std::ifstream inFile(inputPath);
    ASSERT_TRUE(inFile.is_open()) << "Failed to open input file: " << inputRel;
    std::ostringstream inBuf;
    inBuf << inFile.rdbuf();
    std::string inputData = inBuf.str();

    std::ifstream outFile(expectedPath);
    ASSERT_TRUE(outFile.is_open())
        << "Failed to open expected‐output file: " << expectedRel;
    std::ostringstream outBuf;
    outBuf << outFile.rdbuf();
    std::string expectedOutput = outBuf.str();

    std::istringstream codeStream(code);
    std::istringstream inputStream(inputData);
    std::ostringstream actualOut;
    bool ok = interpret(codeStream, inputStream, actualOut);
    ASSERT_TRUE(ok) << "Interpreter returned false on `" << solutionRel << "`";

    std::string actualOutput = actualOut.str();

    ASSERT_EQ(actualOutput, expectedOutput)
        << "Mismatch between actual output and expected output.\n"
           "----- Actual Output -----\n"
        << actualOutput << "\n----- Expected Output -----\n"
        << expectedOutput;
}

TEST(CodeforcesTestSuite, CF_453988E) {
    runCodeforcesTest("tests/etc/treap/solution.itmo",
                      "tests/etc/treap/input.txt",
                      "tests/etc/treap/output.txt");
}
