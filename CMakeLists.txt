cmake_minimum_required(VERSION 3.28)
project(lcp LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall -Wextra 
)

# GTK 4
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Client target
set(CLIENT_SOURCES 
    src/client/main.c
    src/client/client.c
    src/client/peer_list.c
    src/client/gui.c
    src/client/net_callbacks.c

    src/net_shared.c
    src/debug.c
    src/packet_queue.c
    src/packets.c
)
add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PRIVATE include ${GTK4_INCLUDE_DIRS})
target_link_libraries(client PRIVATE ${GTK4_LIBRARIES})
target_compile_options(client PRIVATE ${GTK4_CFLAGS_OTHER})
target_link_directories(client PRIVATE ${GTK4_LIBRARY_DIRS})

# Server target
set(SERVER_SOURCES
    src/server/main.c
    src/server/server.c
    src/server/client_list.c
    src/server/client_context.c
    src/server/net_callbacks.c
    
    src/net_shared.c
    src/debug.c
    src/packet_queue.c
    src/packets.c
)
add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE include)
