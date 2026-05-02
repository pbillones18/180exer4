#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>


#define MAX_SLAVES 16
#define ACK_MSG "ack"

typedef struct {
    char ip[50];
    int port;
} SlaveInfo;

// Function to measure time difference in seconds
double get_elapsed_time(struct timeval t1, struct timeval t2) {
    return (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
}

void master_logic(int n, int master_port, char *config_file) {
    struct timeval t1, t2;
    SlaveInfo slaves[MAX_SLAVES];
    int t = 0;

    // 1. Create Matrix M
    int **M = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        M[i] = (int *)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) {
            M[i][j] = (rand() % 100) + 1; // Random non-zero positive integers
        }
    }

    // 2. Read Configuration File
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        perror("Config file error");
        exit(1);
    }
    while (fscanf(fp, "%s %d", slaves[t].ip, &slaves[t].port) != EOF) {
        t++;
    }
    fclose(fp);

    printf("Master: Found %d slaves in config.\n", t);

    gettimeofday(&t1, NULL); // time_before

    // 3. Divide and Distribute
    int rows_per_slave = n / t;
    for (int i = 0; i < t; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in slave_addr;
        slave_addr.sin_family = AF_INET;
        slave_addr.sin_port = htons(slaves[i].port);
        inet_pton(AF_INET, slaves[i].ip, &slave_addr.sin_addr);

        printf("Master: Connecting to Slave %d at %s:%d...\n", i, slaves[i].ip, slaves[i].port);
        
        // Wait until slave is ready to accept
        while (connect(sock, (struct sockaddr *)&slave_addr, sizeof(slave_addr)) < 0) {
            usleep(100000); // Retry every 100ms
        }

        // Send rows for this submatrix
        int start_row = i * rows_per_slave;
        for (int r = start_row; r < start_row + rows_per_slave; r++) {
            send(sock, M[r], n * sizeof(int), 0);
        }

        // Receive ACK
        char buffer[10];
        recv(sock, buffer, sizeof(buffer), 0);
        if (strcmp(buffer, ACK_MSG) == 0) {
            printf("Master: Received ACK from Slave %d\n", i);
        }

        close(sock);
    }

    gettimeofday(&t2, NULL); // time_after

    printf("\n[MASTER] n=%d, t=%d\n", n, t);
    printf("Time Elapsed: %lf seconds\n", get_elapsed_time(t1, t2));

    // Cleanup
    for (int i = 0; i < n; i++) free(M[i]);
    free(M);
}

void slave_logic(int n, int my_port) {
    struct timeval t1, t2;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(my_port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    listen(server_fd, 3);
    printf("Slave: Listening on port %d...\n", my_port);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept failed");
        exit(1);
    }

    gettimeofday(&t1, NULL); // time_before

    // Allocate memory for receiving submatrix
    // Note: Slave doesn't know 't', but it knows 'n'. Master sends rows_per_slave rows.
    // For simplicity in this lab, we assume n is divisible by t.
    // In a real scenario, Master would send the row count first.
    // We will allocate based on the expected chunk size.
    // To be safe, let's just receive what's coming.
    
    int expected_rows = 0; // This would ideally be received from master
    // For the sake of the lab experiment, let's calculate based on common t values
    // but in practice, receiving raw bytes until connection closes is safer.
    
    int *row_buffer = (int *)malloc(n * sizeof(int));
    ssize_t bytes_received;
    while ((bytes_received = recv(new_socket, row_buffer, n * sizeof(int), 0)) > 0) {
        // Submatrix row received
    }

    // Send ACK
    send(new_socket, ACK_MSG, strlen(ACK_MSG) + 1, 0);

    gettimeofday(&t2, NULL); // time_after

    printf("[SLAVE] Port %d: Submatrix received fully.\n", my_port);
    printf("Time Elapsed: %lf seconds\n", get_elapsed_time(t1, t2));

    free(row_buffer);
    close(new_socket);
    close(server_fd);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <n> <port> <status (0=master, 1=slave)> [config_file]\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int p = atoi(argv[2]);
    int s = atoi(argv[3]);
    char *config = (argc == 5) ? argv[4] : "config.txt";

    srand(time(NULL));

    if (s == 0) {
        master_logic(n, p, config);
    } else {
        slave_logic(n, p);
    }

    return 0;
}