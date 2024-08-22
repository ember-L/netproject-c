# set(netproject_utils 
#     acceptor.c
#     buffer.c 
#     channel.c
#     channel_map.c 
#     event_loop.c
#     event_loop_thread.c
#     event_dispatcher.c  
#     log.c
#     tcp_utils.c
#     tcp_connection.c
#     thread_pool.c
# )

file(GLOB_RECURSE netproject_utils ${PROJECT_SOURCE_DIR}/utils/*.c)

add_library(netutils SHARED ${netproject_utils}) #创建静态库
target_link_libraries(netutils pthread rt)
install(TARGETS netutils DESTINATION lib)