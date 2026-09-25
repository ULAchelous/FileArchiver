# ---- 自动收集测试源文件（新增 test_*.cpp 自动纳入）----
file(GLOB FILEARCHIVER_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
)

add_executable(filearchiver_tests
    ${FILEARCHIVER_TEST_SOURCES}
)

target_link_libraries(filearchiver_tests PRIVATE filearchiver::core)

# 后续接入 Catch2 / GoogleTest 时，在此处通过 FetchContent 拉取并替换下方裸测试
add_test(NAME ArchiverSmokeTest COMMAND filearchiver_tests)
