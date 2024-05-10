#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024

long long current_time_microseconds()
{
    struct timeval te;
    gettimeofday(&te, NULL);                                     // get current time
    long long microseconds = te.tv_sec * 1000000LL + te.tv_usec; // calculate microseconds
    return microseconds;
}

int main(int argc, char *argv[])
{
    printf("Arguments provided:\n");
    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    // Check if the correct number of arguments is provided
    if (argc != 4)
    {
        perror("Give receiver address, port, packet quantity");
        return 1;
    }
    // Parse receiver address and port from command line arguments
    int receiverAddressArg = inet_addr(argv[1]);
    int receiverPortArg = (int)strtol(argv[2], NULL, 0);
    printf("%d\n", receiverAddressArg);
    printf("%d\n", receiverPortArg);

    int buf_size = atoi(argv[3]);
    if (buf_size <= 0)
        buf_size = BUFFER_SIZE;
    char buf[buf_size];
    memset(buf, 'A', buf_size);
    clock_t start, end;
    double cpu_time_used;
    struct sockaddr_in localAddress = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(0)};
    struct sockaddr_in receiverAddress = {.sin_family = AF_INET, .sin_addr.s_addr = receiverAddressArg, .sin_port = htons(receiverPortArg)};
    long long start_time, end_time, sending_time; // Time measurement vars
    // Create a TCP socket
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor < 0)
    {
        perror("Error while creating a socket!");
        return 1;
    }

    // Bind the socket to a local address
    struct sockaddr_in serverAddress = {.sin_family = AF_INET, .sin_addr.s_addr = receiverAddressArg, .sin_port = htons(receiverPortArg)};
    if (connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Error while connecting to the server");
        close(socketDescriptor);
        return 1;
    }

    // Set timeout for the socket
    struct timeval timeout;
    timeout.tv_sec = 100; // 1 second timeout
    timeout.tv_usec = 0;
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("Error setting socket timeout");
        close(socketDescriptor);
        return 1;
    }

    // Send random bytes 10 times
    srand(time(NULL));

    for (int i = 0; i < atol(argv[3]); i++)
    {
        unsigned int buf_size = atoi(argv[3]);
        if (buf_size <= 0)
            buf_size = BUFFER_SIZE;
        char buf[buf_size];
        memset(buf, 'A', buf_size);
        clock_t start, end;
        double cpu_time_used;

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
        start_time = current_time_microseconds(); // Start time measure
        int bytesSent = send(socketDescriptor, buf, sizeof(buf), 0);
        end_time = current_time_microseconds(); // End time measure
        sending_time = end_time - start_time;
        if (bytesSent < 0)
        {
            perror("Error while sending data!");
            close(socketDescriptor);
            return 1;
        }
        printf("Sent %d bytes to receiver (Packet number: %d)!\n Time: %d μs \n ", bytesSent, i, sending_time);
        char recv_buffer[BUFFER_SIZE];
        memset(recv_buffer, 0, BUFFER_SIZE);

        ssize_t bytes_received = recv(socketDescriptor, recv_buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("recv failed");
        } else if (bytes_received == 0) {
            printf("Server closed the connection.\n");
        } else {
            printf("Message received from server: %s\n", recv_buffer);
        }
    }

            // Close the socket
            close(socketDescriptor);

            return 0;
}
