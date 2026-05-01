#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

/**
 * Function to print the submatrix partition.
 * This replaces the old thread function.
 */
void process_submatrix(int **X, int m, int n, int index, int starting_index) {
    printf("\n--- Submatrix %d ---\n", index);
    printf("Rows: %d, Starting Row Index: %d\n", m, starting_index);
    
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < n; col++) {
            printf("%4d ", X[starting_index + row][col]);
        }
        printf("\n");
    }
}

int main() {
    int n, t, rows, columns;
    struct timeval t1, t2;
    double elapsedTime;

    // Seed the random number generator
    srand(time(NULL));

    printf("Enter n: ");
    if (scanf("%d", &n) != 1) return 1;
    printf("Enter t: ");
    if (scanf("%d", &t) != 1) return 1;

    printf("\nMatrix Size (n): %d\n", n);
    printf("Number of Partitions (t): %d\n", t);

    rows = n;
    columns = n;

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

    // (3) Partitioning and processing (Sequential Version)
    gettimeofday(&t1, NULL);

    int starting_index = 0;
    for (int i = 0; i < t; i++) {
        // Using your algorithm: Divide n rows into t parts, handling remainders
        int sub_rows = (n / t + (i < n % t ? 1 : 0));
        
        // Process each submatrix directly in the loop instead of a thread
        process_submatrix(arr, sub_rows, n, i, starting_index);
        
        // Update starting index for the next partition
        starting_index += sub_rows;
    }

    gettimeofday(&t2, NULL);

    // Calculate time elapsed
    elapsedTime = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    printf("\nExecution completed.\n");
    printf("Time elapsed: %lf seconds\n", elapsedTime);

    // (4) Free the dynamically allocated memory
    for (int i = 0; i < rows; i++) {
        free(arr[i]);
    }
    free(arr);

    return 0;
}