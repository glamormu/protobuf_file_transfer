project(secftServer)

aux_source_directory("." SERVER_SRC)
add_library(secftserver SHARED ${SERVER_SRC})
TARGET_LINK_LIBRARIES(secftserver proto ${PROTOBUF_LIBRARY} boost_system pthread boost_thread stdc++fs)
