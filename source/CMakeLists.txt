# In source/CMakeLists.txt
add_executable(uconf_parser_test
    tests/uconf_parser_test.cpp
    core/config/uconf_parser.cpp
)

target_include_directories(uconf_parser_test PRIVATE core/config)