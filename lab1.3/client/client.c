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
    int receiverAddressArg = inet_addr(argv[1]);
    int receiverPortArg = (int)strtol(argv[2], NULL, 0);
    printf("%d\n", receiverAddressArg);
    printf("%d\n", receiverPortArg);

    char buf[BUFFER_SIZE];
    struct sockaddr_in localAddress = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(0)};
    struct sockaddr_in receiverAddress = {.sin_family = AF_INET, .sin_addr.s_addr = receiverAddressArg, .sin_port = htons(receiverPortArg)};
    long int udelay = atol(argv[4]) * 1000;

    // Create a UDP socket
    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor < 0) {
        perror("Error while creating a socket!");
        return 1;
    }

    // Bind the socket to a local address
    int bindResult = bind(socketDescriptor, (const struct sockaddr *)&localAddress, sizeof(localAddress));
    if (bindResult < 0) {
        perror("Error while binding a socket!");
        close(socketDescriptor);
        return 1;
    }

    // Set timeout for the socket
    struct timeval timeout;
    timeout.tv_sec = 2;  // 1 second timeout
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
        for (int j = 8; j < sizeof(buf); j++) // Start from byte 8
        {
            buf[j] = rand() % 256;
        }

        // Send data to the receiver
        int bytesSent = (int)sendto(socketDescriptor, buf, sizeof(buf), 0, (const struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
        if (bytesSent < 0) {
            perror("Error while sending data!");
            close(socketDescriptor);
            return 1;
        }
        printf("Sent %d bytes to receiver (Packet number: %d)!\n", bytesSent, i);

        char ackBuf[BUFFER_SIZE];
        ssize_t bytesReceived = recvfrom(socketDescriptor, ackBuf, sizeof(ackBuf), 0, NULL, NULL);

        if (bytesReceived > 0) {
            int32_t ackNumber;
            memcpy(&ackNumber, ackBuf, sizeof(ackNumber));
            ackNumber = ntohl(ackNumber);
            printf("Received ACK for packet number: %d\n", ackNumber);
        } else if (bytesReceived == 0) {
            printf("Connection closed by server\n");
            close(socketDescriptor);
            return 1;
        } else {
            // Handle timeout (ACK not received within the specified time)
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("Timeout! Resending packet...\n");
                i--;  // Resend the packet
            } else {
                perror("Error while receiving ACK");
                close(socketDescriptor);
                return 1;
            }
        }
        usleep(udelay);
        
    }

    // Close the socket
    close(socketDescriptor);

    return 0;
}

