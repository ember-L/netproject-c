add_executable(epoll-server-onethread epoll-server-onethread.c)

target_link_libraries(epoll-server-onethread netutils)

add_executable(epoll-server-multithreads epoll-server-multithreads.c)

target_link_libraries(epoll-server-multithreads netutils)