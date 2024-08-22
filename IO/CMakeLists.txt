add_executable(select select.c)
target_link_libraries(select netutils)

add_executable(poll poll.c)
target_link_libraries(poll netutils)

add_executable(epoll epoll.c)
target_link_libraries(epoll netutils)