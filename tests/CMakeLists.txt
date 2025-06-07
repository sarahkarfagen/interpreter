include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
  itmoscript_tests
  # basic_test.cpp
  stdlib_test.cpp
  function_test.cpp
  types_test.cpp
  illegal_ops_test.cpp
  loop_and_branch_test.cpp
  #codeforces_test.cpp
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.experimental")
    file(GLOB OPT_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/.experimental/*.cpp")
    list(APPEND TEST_SOURCES ${OPT_TESTS})
endif()

add_executable(
  itmoscript_tests
  ${TEST_SOURCES}
)

target_compile_definitions(itmoscript_tests
    PRIVATE
        PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
  itmoscript_tests
  itmoscript
  GTest::gtest_main
)

target_include_directories(itmoscript_tests PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)

gtest_discover_tests(itmoscript_tests)