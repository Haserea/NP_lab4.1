#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib") // Link the Winsock library

#define DEFAULT_PORT 5000

SOCKET server_socket = INVALID_SOCKET;

void free_socket() {
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
}

void usage(const char* exe_name) {
    printf("Usage:\n");
    printf("\t%s [-p <port>]\n", exe_name);
}

int start(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        if (sscanf_s(argv[2], "%d", &port) != 1 || port <= 0) {
            printf("Invalid port number\n");
            usage(argv[0]);
            return -1;
        }
    }

    return init_server(port);
}

int init_server(short port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock\n");
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket == INVALID_SOCKET) {
        printf("Cannot create socket: %ld\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    struct sockaddr_in server_address = { 0 };
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Cannot bind socket to port %d: %ld\n", port, WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return -2;
    }

    printf("Server is running on port %d\n", port);

    int result = process_requests();
    free_socket();
    WSACleanup();
    return result;
}

static int process_requests() {
    struct sockaddr_in client_addr = { 0 };
    int len = sizeof(client_addr);
    char buffer[1024] = { 0 };

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int ret = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client_addr, &len);

        if (ret <= 0) {
            printf("Receiving data error: %ld\n", WSAGetLastError());
            continue;
        }

        buffer[ret] = '\0'; // Null-terminate the received data
        printf("Received request '%s' from %s\n", buffer, inet_ntoa(client_addr.sin_addr));

        time_t now = time(NULL);
        char response[1024];
        strftime(response, sizeof(response), "%Y-%m-%d %H:%M:%S", localtime(&now));

        printf("Sending current time: %s\n", response);

        ret = sendto(server_socket, response, strlen(response), 0, (struct sockaddr*)&client_addr, len);

        if (ret <= 0) {
            printf("Sending response error: %ld\n", WSAGetLastError());
        }
    }

    return 0;
}