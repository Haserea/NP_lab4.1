#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib") // Link the Winsock library

#define DEFAULT_PORT 5000

void usage(const char* exe_name) {
    printf("Usage:\n");
    printf("\t%s -h <host> -p <port>\n", exe_name);
}

int start(int argc, char* argv[]) {
    char host[2048] = "";
    int port = DEFAULT_PORT;

    if (argc >= 5 && strcmp(argv[1], "-h") == 0 && strcmp(argv[3], "-p") == 0) {
        if (sscanf_s(argv[2], "%s", host, _countof(host)) != 1) {
            printf("Invalid host address\n");
            usage(argv[0]);
            return -1;
        }

        if (sscanf_s(argv[4], "%d", &port) != 1 || port <= 0) {
            printf("Invalid port number\n");
            usage(argv[0]);
            return -1;
        }
    }
    else {
        printf("Enter server address (-h <host> -p <port>): ");
        if (scanf_s("%s %d", host, _countof(host), &port) != 2) {
            printf("Invalid input\n");
            usage(argv[0]);
            return -1;
        }
    }

    return init_client(host, port);
}

int init_client(const char* host, short port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock\n");
        return -1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Cannot create client socket: %ld\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    printf("Socket created\n");

    struct sockaddr_in server_address = { 0 };
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
        printf("Invalid server address\n");
        closesocket(client_socket);
        WSACleanup();
        return -2;
    }

    printf("Connecting to server: %s:%d\n", host, port);

    int result = process_connection(client_socket, server_address);
    closesocket(client_socket);
    WSACleanup();
    return result;
}

int process_connection(SOCKET client_socket, struct sockaddr_in server_address) {
    char buffer[1024] = "";

    // Send a non-empty message to trigger the server
    const char* message = "ping";
    int ret = sendto(client_socket, message, strlen(message), 0, (struct sockaddr*)&server_address, sizeof(server_address));
    if (ret <= 0) {
        printf("Sending data error: %ld\n", WSAGetLastError());
        return -11;
    }

    printf("Request sent to server\n");

    memset(buffer, 0, sizeof(buffer));

    struct sockaddr_in from_address = { 0 };
    int from_len = sizeof(from_address);
    ret = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&from_address, &from_len);

    if (ret <= 0) {
        printf("Receiving data error: %ld\n", WSAGetLastError());
        return -12;
    }

    buffer[ret] = '\0'; // Null-terminate the received data
    printf("Server time: %s\n", buffer);

    return 0;
}