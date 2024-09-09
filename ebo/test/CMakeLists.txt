do_test(ebo_server subprocess_test "")
do_test(ebo_server log_test "")
install(TARGETS log_test
        DESTINATION test)

do_test(ebo_server bind_test "")
install(TARGETS bind_test
        DESTINATION test)

do_test(ebo_server http_parser_test "")
do_test(ebo_server url_parser_test "")
do_test(ebo_server sqlite3_client_test "")
install(TARGETS sqlite3_client_test
        DESTINATION test)

do_test(ebo_server mysql_client_test "")
do_test(ebo_server encryptor_test "")
