#include <stdio.h>        // For printf
#include <winsock2.h>     // For Winsock functions
#include <ws2tcpip.h>     // For AF_INET and sockaddr_in

#pragma comment(lib, "Ws2_32.lib")  // Link with Winsock library

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
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

    // Set socket options
    // Allows for the listening socket to rebind quickly if the listening socket closes
    int option = 1;  // Enable the option
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option)) < 0) {
        printf("setsockopt failed! Error code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Socket option SO_REUSEADDR set successfully.\n");

    // Set up the address structure
    struct sockaddr_in address;
    int port_number = 8080;
    address.sin_family = AF_INET;           // IPv4
    address.sin_port = htons(port_number);  // Port number in network byte order
    address.sin_addr.s_addr = INADDR_ANY;   // Bind to all available interfaces
    socklen_t address_byte_size = sizeof(address);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&address, address_byte_size) == SOCKET_ERROR) {
        printf("Bind failed! Error code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Bind successful!\n");

    // Listen on the socket
    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed! Error code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Listening on socket...\n");

    unsigned long mode = 1;  // 1 to enable non-blocking mode
    if (ioctlsocket(sockfd, FIONBIO, &mode) != 0) {
        printf("Failed to set non-blocking mode! Error code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Socket set to non-blocking mode.\n");

    fd_set readfds;
    struct timeval timeout;

    // Set a timeout 
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int activity = 0;
    int new_sockfd = INVALID_SOCKET;
    int MAX_NUM_RETRIES = 6;

    // Set the number of retries to 6 to avoid an infinite loop. 
    for (int retries = 0; retries < MAX_NUM_RETRIES && activity == 0; retries++) {
        printf("Attempt %d to accept a connection...\n", retries + 1);

        // Initialize the file descriptor set every time the select statement is called
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0 && WSAGetLastError() != WSAEINTR) {
            printf("Select failed! Error code: %d\n", WSAGetLastError());
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }
        else if (activity == 0) {
            // Timeout occurred, no incoming connection
            printf("No connections received within the timeout period.\n");
        }
        else if (FD_ISSET(sockfd, &readfds)) {
            // Socket is readable, accept the connection
            new_sockfd = accept(sockfd, (struct sockaddr*)&address, &address_byte_size);
            if (new_sockfd == INVALID_SOCKET) {
                printf("Accept failed! Error code: %d\n", WSAGetLastError());
                closesocket(new_sockfd);
                closesocket(sockfd);
                WSACleanup();
                return 1;
            }
            char ip_string[INET_ADDRSTRLEN]; // Buffer to hold the IP address as a string                
            if (InetNtop(AF_INET, &(address.sin_addr), ip_string, INET_ADDRSTRLEN) == NULL) {
                printf("Failed to convert IP address to string! Error code: %d\n", WSAGetLastError());
            }
            else {
                printf("Connection accepted from %s:%d\n", ip_string, ntohs(address.sin_port));
            }
        }
    }

    // Clean up sockets
    closesocket(new_sockfd);  // Close the connection socket
    closesocket(sockfd);      // Close the listening socket
    WSACleanup();             // It always important to cleanup Winsock resources so memory leaks can be prevented
    return 0;
}
