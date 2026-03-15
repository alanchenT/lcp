#include "debug.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

#include "net_shared.h"

void display_peer_info(int socket_fd) {
    struct sockaddr server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    int status = getpeername(socket_fd, &server_addr, &server_addr_len);
    if (status == -1) {
        perror("[display_peer_info] getpeername error");
        return;
    }

    printf("socket %d - peer:\n", socket_fd);

    char ip_string[INET6_ADDRSTRLEN];
    char host_name[NI_MAXHOST];
    char service_name[NI_MAXSERV];

    void* sockaddr = extract_sockaddr(&server_addr);

    // Extract stringified IP address
    const char* ntop_status = inet_ntop(server_addr.sa_family, sockaddr, ip_string, sizeof(ip_string));
    if (ntop_status == NULL) {
        perror("inet_ntop error");
        return;
    }

    if (server_addr.sa_family == AF_INET) {
        printf("\tipv4: %s\n", ip_string);
    }

    status = getnameinfo(
        &server_addr,
        server_addr_len,
        host_name,
        sizeof(host_name),
        service_name,
        sizeof(service_name),
        NI_NAMEREQD
    );

    if (status == -1) {
        perror("getnameinfo error");
        return;
    }

    printf("\thost name: %s\n", host_name);
    printf("\tservice name: %s\n", service_name);
}
