cmake_minimum_required(VERSION 3.16)

project(okkhor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

# ------------------------------------------------------------
# Data files
# ------------------------------------------------------------

set(OKKHOR_DEFAULT_DATA_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/data/default"
)

set(OKKHOR_DATA_FILES
    "${OKKHOR_DEFAULT_DATA_DIR}/vowels.json"
    "${OKKHOR_DEFAULT_DATA_DIR}/consonants.json"
    "${OKKHOR_DEFAULT_DATA_DIR}/controls.json"
    "${OKKHOR_DEFAULT_DATA_DIR}/punctuation.json"
    "${OKKHOR_DEFAULT_DATA_DIR}/rules.json"
)

set_property(
    DIRECTORY APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${OKKHOR_DATA_FILES}
)

# ------------------------------------------------------------
# Generate embedded default data header
# ------------------------------------------------------------

set(OKKHOR_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

set(OKKHOR_DEFAULT_DATA_HPP
    "${OKKHOR_GENERATED_DIR}/default_data.hpp"
)

file(READ
    "${OKKHOR_DEFAULT_DATA_DIR}/vowels.json"
    OKKHOR_VOWELS_JSON
)

file(READ
    "${OKKHOR_DEFAULT_DATA_DIR}/consonants.json"
    OKKHOR_CONSONANTS_JSON
)

file(READ
    "${OKKHOR_DEFAULT_DATA_DIR}/controls.json"
    OKKHOR_CONTROLS_JSON
)

file(READ
    "${OKKHOR_DEFAULT_DATA_DIR}/punctuation.json"
    OKKHOR_PUNCTUATION_JSON
)

file(READ
    "${OKKHOR_DEFAULT_DATA_DIR}/rules.json"
    OKKHOR_RULES_JSON
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/default_data.hpp.in"
    "${OKKHOR_DEFAULT_DATA_HPP}"
    @ONLY
)

# ------------------------------------------------------------
# Core library
# ------------------------------------------------------------

add_library(okkhor_core
    src/core/engine.cpp
    src/core/mapping.cpp
    src/core/orthography.cpp
    src/core/tokenizer.cpp

    src/util/json.cpp

    src/latin_to_bangla/parser.cpp
    src/latin_to_bangla/renderer.cpp
    src/latin_to_bangla/rules.cpp

    src/bangla_to_latin/parser.cpp
    src/bangla_to_latin/renderer.cpp
    src/bangla_to_latin/rules.cpp

    "${OKKHOR_DEFAULT_DATA_HPP}"
)

target_include_directories(okkhor_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${OKKHOR_GENERATED_DIR}"

    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# ------------------------------------------------------------
# Tests
# ------------------------------------------------------------

enable_testing()

add_executable(
    engine_tests
    tests/engine_tests.cpp
)

target_link_libraries(
    engine_tests
    PRIVATE
        okkhor_core
)

target_include_directories(
    engine_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)

add_test(
    NAME engine
    COMMAND engine_tests
)