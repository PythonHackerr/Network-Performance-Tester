#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 1024
int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 5) {
        perror("Give receiver address, port, packet quantity and delay");
        return 1;
    }
    // Parse receiver address and port from command line arguments
    struct in6_addr receiverAddressArg;
    if (inet_pton(AF_INET6, argv[1], &receiverAddressArg) != 1) {
        perror("Invalid IPv6 address");
        return 1;
    }

    int receiverPortArg = (int)strtol(argv[2], NULL, 0);
    printf("%s\n", receiverAddressArg);
    printf("%d\n", receiverPortArg);

    char buf[BUFFER_SIZE];
    struct sockaddr_in6 localAddress = {.sin6_family = AF_INET6, .sin6_addr = in6addr_any, .sin6_port = htons(0)};
    struct sockaddr_in6 receiverAddress = {.sin6_family = AF_INET6, .sin6_addr = receiverAddressArg, .sin6_port = htons(receiverPortArg)};

    // Create a TCP socket
    int socketDescriptor = socket(AF_INET6, SOCK_STREAM, 0);
    if (socketDescriptor < 0) {
        perror("Error while creating a socket!");
        return 1;
    }

    // Bind the socket to a local address
    struct sockaddr_in6 serverAddress = {.sin6_family = AF_INET6, .sin6_addr = receiverAddressArg, .sin6_port = htons(receiverPortArg)};
    if (connect(socketDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error while connecting to the server");
        close(socketDescriptor);
        return 1;
    }

    // Set timeout for the socket
    struct timeval timeout;
    timeout.tv_sec = 1;  // 1 second timeout
    timeout.tv_usec = 0;
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Error setting socket timeout");
        close(socketDescriptor);
        return 1;
    }

    // Send random bytes 10 times
    srand(time(NULL));

    for (int i = 0; i < atol(argv[3]); i++) {
        unsigned char buf[BUFFER_SIZE];

        // Data sequence number
        int32_t packetNumber = htonl(i);
        memcpy(buf, &packetNumber, sizeof(packetNumber));

        // Datagram Length
        int16_t packetLength = htons(sizeof(buf)); // Length of the entire buffer
        memcpy(buf + sizeof(packetNumber), &packetLength, sizeof(packetLength));

        // Random bytes
        for (int j = sizeof(packetNumber); j < sizeof(buf); j++) // Start from byte 8
        {
            buf[j] = rand() % 256;
        }

        // Send data to the receiver
        int bytesSent = send(socketDescriptor, buf, sizeof(buf), 0);
        if (bytesSent < 0) {
            perror("Error while sending data!");
            close(socketDescriptor);
            return 1;
        }
        printf("Sent %d bytes to receiver (Packet number: %d)!\n", bytesSent, i);
    }

    // Close the socket
    close(socketDescriptor);

    return 0;
}

