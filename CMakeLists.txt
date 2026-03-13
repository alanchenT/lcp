cmake_minimum_required(VERSION 3.28)
project(lcp LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# Client target
set(CLIENT_SOURCES 
    src/client/main.c
    src/client/net.c

    src/net_shared.c
    src/debug.c
    src/protocol.c
    src/send_queue.c
)
add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PRIVATE include)

# Server target
set(SERVER_SOURCES
    src/server/main.c
    src/server/net.c
    src/server/client_list.c
    
    src/net_shared.c
    src/debug.c
    src/protocol.c
    src/send_queue.c
)
add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE include)
