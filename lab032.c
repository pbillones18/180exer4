#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Function to print the submatrix partition.
 * This replaces the old thread function.
 */
// void process_submatrix(int **X, int m, int n, int index, int starting_index) {
//     printf("\n--- Submatrix %d ---\n", index);
//     printf("Rows: %d, Starting Row Index: %d\n", m, starting_index);
    
//     for (int row = 0; row < m; row++) {
//         for (int col = 0; col < n; col++) {
//             printf("%4d ", X[starting_index + row][col]);
//         }
//         printf("\n");
//     }
// }

typedef struct {
    char ip[50];
    int port;
} SlaveInfo;

int main(int argc, char *argv[]) {
    // int n, t, rows, columns;
    int n, rows, columns;
    // int t = 2;
    struct timeval t1, t2;
    double elapsedTime;
    int t;

    SlaveInfo slaves[100];

    // Seed the random number generator
    srand(time(NULL));

    if (argc != 4) {
        printf("Usage: %s <n> <port> <status>\n", argv[0]);
        printf("status: 0 = master, 1 = slave\n");
        return 1;
    }

    n = atoi(argv[1]);
    int port = atoi(argv[2]);
    int status = atoi(argv[3]);

    printf("\nMatrix Size (n): %d\n", n);
    FILE *fp = fopen("config.txt", "r");

    if (fp == NULL) {
        perror("config file failed");
        return 1;
    }

    fscanf(fp, "%d", &t);

    for (int i = 0; i < t; i++) {

        fscanf(fp,
            "%s %d",
            slaves[i].ip,
            &slaves[i].port);
    }

    fclose(fp);

    printf("\nNumber of slaves: %d\n", t);

    for (int i = 0; i < t; i++) {

        printf("Slave %d -> %s:%d\n",
            i,
            slaves[i].ip,
            slaves[i].port);
    }
    int socks[2];
    if (status == 0) {

        for (int i = 0; i < t; i++) {

            struct sockaddr_in slave_addr;

            socks[i] = socket(AF_INET, SOCK_STREAM, 0);

            if (socks[i] < 0) {
                perror("socket failed");
                return 1;
            }

            slave_addr.sin_family = AF_INET;

            // slave_addr.sin_port = htons(8081 + i);
            slave_addr.sin_port = htons(slaves[i].port);

            // inet_pton(AF_INET,
            //         "127.0.0.1",
            //         &slave_addr.sin_addr);
            inet_pton(AF_INET,
                slaves[i].ip,
                &slave_addr.sin_addr);

            printf("\nConnecting to slave %d...\n", i);

            if (connect(socks[i],
                    (struct sockaddr*)&slave_addr,
                    sizeof(slave_addr)) < 0) {

                perror("connect failed");
                return 1;
            }

            printf("Connected to slave %d!\n", i);
        }
    }

    rows = n;
    columns = n;


    if (status == 1) {

    int server_fd, client_socket;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr)) < 0) {

        perror("bind failed");
        return 1;
    }

    listen(server_fd, 5);

    printf("\nSlave listening on port %d...\n", port);

    client_socket = accept(server_fd, NULL, NULL);

    if (client_socket < 0) {
        perror("accept failed");
        return 1;
    }

    printf("Master connected!\n");

    // char buffer[100];

    // recv(client_socket, buffer, sizeof(buffer), 0);

    // printf("Received: %s\n", buffer);
    int recv_rows;
    int recv_cols;

    recv(client_socket, &recv_rows, sizeof(int), 0);
    recv(client_socket, &recv_cols, sizeof(int), 0);

    printf("\nReceiving matrix of size %d x %d\n",
        recv_rows,
        recv_cols);

    int **recv_matrix;

    recv_matrix = (int **)malloc(recv_rows * sizeof(int *));

    for (int i = 0; i < recv_rows; i++) {
        recv_matrix[i] = (int *)malloc(recv_cols * sizeof(int));

        recv(client_socket,
            recv_matrix[i],
            recv_cols * sizeof(int),
            0);
    }

    printf("\nReceived Matrix:\n");
    // printf("%d received rows", recv_rows);
    for (int i = 0; i < recv_rows; i++) {
        for (int j = 0; j < recv_cols; j++) {
            printf("%4d", recv_matrix[i][j]);
        }
        printf("\n");
    }

    send(client_socket, "ack", 3, 0);

    close(client_socket);
    close(server_fd);

    return 0;
}


    // (1) Dynamically allocate memory for the main matrix
    int **arr = (int **)malloc(rows * sizeof(int *));
    if (arr == NULL) {
        perror("malloc failed for rows");
        return 1;
    }

    for (int i = 0; i < rows; i++) {
        arr[i] = (int *)malloc(columns * sizeof(int));
        if (arr[i] == NULL) {
            perror("malloc failed for columns");
            return 1;
        }
    }

    // (2) Filling array with random numbers from 1 to 100
    int min = 1;
    int max = 100;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            arr[i][j] = (rand() % (max - min + 1)) + min;
        }
    }

    // Print the full original matrix
    printf("\nOriginal Matrix X of size %d x %d:\n", rows, columns);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%4d", arr[i][j]);
        }
        printf("\n");
    }

    // for (int i = 0; i < rows; i++) {
    //     send(sock, arr[i], columns * sizeof(int), 0);
    // }
    if (status == 0) {

        int starting_row = 0;
        gettimeofday(&t1, NULL);

        for (int s = 0; s < t; s++) {

            int sub_rows =
                n / t + (s < n % t ? 1 : 0);

            printf("\nSending submatrix to slave %d\n", s);

            send(socks[s],
                &sub_rows,
                sizeof(int),
                0);

            send(socks[s],
                &columns,
                sizeof(int),
                0);

            for (int r = 0; r < sub_rows; r++) {

                send(socks[s],
                    arr[starting_row + r],
                    columns * sizeof(int),
                    0);
            }

            starting_row += sub_rows;
        }

        for (int s = 0; s < t; s++) {

            char buffer[100];

            recv(socks[s],
                buffer,
                sizeof(buffer),
                0);

            printf("Slave %d replied: %s\n",
                s,
                buffer);

            close(socks[s]);
            
        }
        gettimeofday(&t2, NULL);
        elapsedTime =
            (t2.tv_sec - t1.tv_sec) +
            (t2.tv_usec - t1.tv_usec) / 1000000.0;

        printf("\nDistribution Time: %lf seconds\n",
            elapsedTime);
    }
    // (3) Partitioning and processing (Sequential Version)
    // gettimeofday(&t1, NULL);

    // int starting_index = 0;
    // for (int i = 0; i < t; i++) {
    //     // Using your algorithm: Divide n rows into t parts, handling remainders
    //     int sub_rows = (n / t + (i < n % t ? 1 : 0));
        
    //     // Process each submatrix directly in the loop instead of a thread
    //     process_submatrix(arr, sub_rows, n, i, starting_index);
        
    //     // Update starting index for the next partition
    //     starting_index += sub_rows;
    // }

    // gettimeofday(&t2, NULL);

    // // Calculate time elapsed
    // elapsedTime = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    // printf("\nExecution completed.\n");
    // printf("Time elapsed: %lf seconds\n", elapsedTime);

    // (4) Free the dynamically allocated memory
    for (int i = 0; i < rows; i++) {
        free(arr[i]);
    }
    free(arr);

    return 0;
}