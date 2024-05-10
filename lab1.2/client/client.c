#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 3) {
        perror("Give address and port");
        return 1;
    }

    // Parse receiver address and port from command line arguments
    int receiverAddressArg = inet_addr(argv[1]);
    int receiverPortArg = (int)strtol(argv[2], NULL, 0);

    char buf[BUFFER_SIZE];
    struct sockaddr_in localAddress = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(0)};
    struct sockaddr_in receiverAddress = {.sin_family = AF_INET, .sin_addr.s_addr = receiverAddressArg, .sin_port = htons(receiverPortArg)};
    int addrLength = sizeof(receiverAddress);
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

    // Send random bytes 10 times
    srand(time(NULL));

    for (int i = 0; i < 10; i++) {
        unsigned char buf[BUFFER_SIZE * (i + 1)];
        //unsigned char buf[BUFFER_SIZE * 255 + i];

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
        printf("Want to send: %d\n", sizeof(buf));
        int bytesSent = (int)sendto(socketDescriptor, buf, sizeof(buf), 0, (const struct sockaddr *)&receiverAddress, addrLength);
        if (bytesSent < 0) {
            perror("Error while sending data!" );
            close(socketDescriptor);
            return 1;
        }
        printf("Sent %d bytes to receiver (Packet number: %d)!\n", bytesSent, i);

        //Receive response
        memset(buf, 0, sizeof(buf));
        int response = recvfrom(socketDescriptor, buf, sizeof(buf), 0, (struct sockaddr *) &receiverAddress, &addrLength);
        if (response == -1) {
            perror("Not received a response");
            exit(1);
        }
        else{
            printf("Received response %d\n", response);
        }

    }

    // Close the socket
    close(socketDescriptor);

    return 0;
}
