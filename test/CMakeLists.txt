# テスト用のCMakeLists.txt

# Google Testが見つからない場合は自動ダウンロード
find_package(GTest QUIET)
if(GTest_FOUND)
    message(STATUS "Found Google Test")
    include(GoogleTest)
else()
    message(STATUS "Google Test not found, downloading...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50e46cd4995ad50c0ec9a0d2a9da8e.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# STUNパケットテスト
add_executable(test_stun_packet test_stun_packet.cpp
    ../src/stun_packet.cpp
)
target_link_libraries(test_stun_packet 
    udp_hole_punching_lib 
    gtest_main
    gtest
    pthread
)
gtest_discover_tests(test_stun_packet)

# 全テストを実行するカスタムターゲット
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    COMMENT "Running all tests"
)
