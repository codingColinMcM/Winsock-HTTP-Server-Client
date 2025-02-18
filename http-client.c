#include <stdio.h>        // For printf
#include <winsock2.h>     // For Winsock functions
#include <ws2tcpip.h>     // For AF_INET and sockaddr_in

#pragma comment(lib, "Ws2_32.lib")  // Link with Winsock library

int main() {

    // Initialize Winsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Setting to Winsock version 2.2 in the first parameter
    if (wsaResult != 0) {
        printf("WSAStartup failed: %d\n", wsaResult);
        return 1;
    }

    int domain = AF_INET;         // Use IPv4
    int type = SOCK_STREAM;       // TCP (connection-oriented)
    int protocol = 0;             // Use default protocol (IP)

    // Create the socket
    int sockfd = socket(domain, type, protocol);

    // Check if socket creation was successful
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed! Error code: %d\n", WSAGetLastError());
        WSACleanup();  // Clean up Winsock
        return 1;
    }
    printf("Socket created successfully!\n");

    struct sockaddr_in server_address;
    int port_number = 8080;
    server_address.sin_family = AF_INET;            // IPv4
    server_address.sin_port = htons(port_number);   // Port number in the network byte order
    char local_host_address[20] = "127.0.0.1";      // A future refactor of this project will have the user set the IP address they wish for the client to connect to

    if (inet_pton(AF_INET, local_host_address, &server_address.sin_addr) <= 0) {
        printf("Invalid address/Address not supported.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("The address was validated!");

    size_t address_byte_size = sizeof(server_address);

    if (connect(sockfd, (struct sockaddr*)&server_address, (socklen_t)address_byte_size) == SOCKET_ERROR) {
        printf("Connect failed! Error code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Connected to the server successfully!\n");



    closesocket(sockfd);    // Close connection socket
    WSACleanup();           // It always important to cleanup Winsock resources so memory leaks can be prevented
    return 0;
}
