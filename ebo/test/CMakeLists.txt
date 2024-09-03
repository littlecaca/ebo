do_test(ebo_server subprocess_test "")
do_test(ebo_server log_test "")
install(TARGETS log_test
        DESTINATION test)

do_test(ebo_server bind_test "")
install(TARGETS bind_test
        DESTINATION test)