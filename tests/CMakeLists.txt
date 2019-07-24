cmake_minimum_required(VERSION 3.4)
project(pushover-tests)

add_executable(pushover_test src/pushover_test.c)
target_link_libraries(pushover_test pushover)

