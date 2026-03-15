cmake_minimum_required(VERSION 3.28)
project(lcp LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# Client target
set(CLIENT_SOURCES 
    src/client/main.c
    src/client/client.c

    src/net_shared.c
    src/debug.c
    src/payload_queue.c
    src/packets.c
)
add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PRIVATE include)

# Server target
set(SERVER_SOURCES
    src/server/main.c
    src/server/server.c
    src/server/client_list.c
    src/server/client_context.c
    
    src/net_shared.c
    src/debug.c
    src/payload_queue.c
    src/packets.c
)
add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE include)
