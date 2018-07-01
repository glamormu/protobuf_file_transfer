project(secftServer)

aux_source_directory("." CLIENT_SRC)
add_library(secftclient SHARED ${CLIENT_SRC})
TARGET_LINK_LIBRARIES(secftclient proto ${PROTOBUF_LIBRARY} ssl boost_system pthread stdc++fs)
