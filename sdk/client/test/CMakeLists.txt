project(secft_test)

aux_source_directory("." TEST_SRC)
add_executable(secftclient_test ${TEST_SRC})
target_link_libraries(secftclient_test secftclient)