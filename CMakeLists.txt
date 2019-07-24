cmake_minimum_required(VERSION 3.4)
project(pushover)

option(PUSHOVER_BUILD_TESTS "build tests" OFF)

find_package(CURL REQUIRED COMPONENTS HTTPS)
find_package(cJSON REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})
include_directories(${CJSON_INCLUDE_DIRS})

list(APPEND DEPS
    ${CURL_LIBRARIES}
    ${CJSON_LIBRARIES}
    )

list(APPEND SRCS
    src/pushover.c
    )

add_library(${PROJECT_NAME} STATIC ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEPS})

if (PUSHOVER_BUILD_TESTS)
    add_subdirectory(tests)
endif (PUSHOVER_BUILD_TESTS)

