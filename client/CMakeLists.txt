PROJECT(SecFT)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
SET(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++11")

# 添加include目录，本例头文件在./目录下，所以直接取${PROJECT_SOURCE_DIR}/
include_directories(${PROJECT_SOURCE_DIR}/include)
# 添加链接库查找目录，库查找序按照参数顺序从向右查
link_directories(${PROJECT_BINARY_DIR}/libs /usr.local/libs /usr/lib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)

# 设置可执行文件目录，同样设成和库目录相同
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test)

# 添加子目录（每个子目录下都必须有自已的CMakeLists.txt文件
add_subdirectory(secclient)
add_subdirectory(test)
