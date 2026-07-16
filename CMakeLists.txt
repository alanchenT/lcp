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

# GTK Blueprint and resource compilers
find_program(BLUEPRINT_COMPILER NAMES blueprint-compiler REQUIRED)
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

# Compile GTK Blueprint and resource files
set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(RESOURCE_XML ${RESOURCE_DIR}/gresource.xml)
set(RESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/resources.c)

set(BLP_FILES
    resources/app.blp
)
set(UI_FILES "")

foreach(BLP_FILE IN LISTS BLP_FILES)
    # Extract the filename without extension
    get_filename_component(BASENAME ${BLP_FILE} NAME_WE)
    set(UI_FILE ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.ui)
    
    add_custom_command(
        OUTPUT ${UI_FILE}
        COMMAND ${BLUEPRINT_COMPILER} compile ${CMAKE_CURRENT_SOURCE_DIR}/${BLP_FILE} --output ${UI_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${BLP_FILE}
        COMMENT "Compiling ${BASENAME}.blp to .ui..."
    )
    
    list(APPEND UI_FILES ${UI_FILE})
endforeach()

add_custom_command(
    OUTPUT ${RESOURCE_C}
    WORKING_DIRECTORY ${RESOURCE_DIR}
    COMMAND ${GLIB_COMPILE_RESOURCES} 
            --target=${RESOURCE_C} 
            --generate-source 
            --sourcedir=${RESOURCE_DIR}
            --sourcedir=${CMAKE_CURRENT_BINARY_DIR}
            ${RESOURCE_XML} 
    DEPENDS ${RESOURCE_XML} ${UI_FILES}
    VERBATIM
    COMMENT "Compiling resources..."
)

# Client target
set(CLIENT_SOURCES 
    src/client/app.c
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
add_executable(client ${CLIENT_SOURCES} ${RESOURCE_C})
target_include_directories(client PRIVATE include ${GTK4_INCLUDE_DIRS})
target_link_libraries(client PRIVATE ${GTK4_LIBRARIES})
target_compile_options(client PRIVATE ${GTK4_CFLAGS_OTHER})
target_link_directories(client PRIVATE ${GTK4_LIBRARY_DIRS})

set_source_files_properties(${RESOURCE_C} PROPERTIES GENERATED TRUE)

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
