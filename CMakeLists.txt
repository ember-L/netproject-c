cmake_minimum_required(VERSION 3.1)

project(netproject C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}) 

#将静态文件拷贝到指定文件夹
file(COPY ${PROJECT_SOURCE_DIR}/Http/root DESTINATION ${CMAKE_BINARY_DIR}/bin)

# include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
add_subdirectory(utils)
add_subdirectory(IO)
add_subdirectory(NonBlock)
add_subdirectory(TCP)
add_subdirectory(Concurrent)
add_subdirectory(Http)


