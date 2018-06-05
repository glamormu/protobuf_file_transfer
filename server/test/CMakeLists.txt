project(secft_test)

aux_source_directory("." TEST_SRC)
add_executable(secft_test ${TEST_SRC})
target_link_libraries(secft_test secftserver)