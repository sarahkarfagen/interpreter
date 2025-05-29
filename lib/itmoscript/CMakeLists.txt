file(GLOB ITMO_SOURCES "src/*.cpp")

add_library(itmoscript ${ITMO_SOURCES})

target_include_directories(itmoscript
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
