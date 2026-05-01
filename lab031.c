
#define __USE_GNU
#define _GNU_SOURCE
// #include<stdio.h>
// #include<malloc.h>
// #include<stdlib.h>
// #include<time.h>
// #include<sys/time.h>
// #include<math.h>
// #include<pthread.h>

#include<stdio.h>
#include<malloc.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<math.h>
#include<pthread.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>



#define print_error_then_terminate(en, msg) \
    do { \
        errno = en; \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0)



// Structure for passing arguments to the thread function
struct ThreadArg
{
    int **X;
    int m;
    int n;
    double *r;
    int index;
    int starting_index;
    int q;
};
void *mse_ma2(void *arg){
    struct ThreadArg *data = (struct ThreadArg *)arg;
    int m = data->m;
    int n = data->n;
    int index = data->index;
    int starting_index = data->starting_index;
    int q = data->q;


const int core_id = index % 3;
    const pid_t pid = getpid();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    const int set_result = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    if (set_result != 0)
    {

        print_error_then_terminate(set_result, "sched_setaffinity");
    }

    const int get_affinity = sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset);
    if (get_affinity != 0)
    {

        print_error_then_terminate(get_affinity, "sched_getaffinity");
    }

    // if (CPU_ISSET(core_id, &cpuset))
    // {

    //     fprintf(stdout, "Successfully set thread %ld to affinity to CPU %d\n", (long)gettid(), core_id);
    // }
    // else
    // {

    //     fprintf(stderr, "Failed to set thread %d to affinity to CPU %d\n", pid, core_id);
    // }



    double *v = (double *)malloc(m * sizeof(double));

    // printf("starting index: %d\n", data->starting_index); //PRINTING SUBMATRIX STARTING INDEX

    printf("submatrix %d\n", data->index); //PRINTING SUBMATRIX AND SUBMATRIX NUMBER
    for(int row = 0; row < m; row++){
        for(int col = 0; col < n; col++){
            printf("%d ", data->X[starting_index + row][ col]); // Access correct column based on starting_index
        }
        printf("\n");
    }

    // for each row
    for (int i = 0; i < m; i++) {
        // initialize to store summation of (Xij-MAi(q))^2
        double squared_errors_sum = 0;


        // start at q then move backwards to get average or q previous elements and compare to current element
        // from summation from i=q+1 to m in formula ex. 6 and 7 of 8
        for (int j = q; j < n; j++) {
            
            
            // initialize for Xkj summation
            double current_ma_sum = 0; //j-q ex. 6-6 
            for (int k = j - q; k <= j - 1; k++) {// following the formula MAj(q)
                current_ma_sum += data->X[starting_index+i][k]; // Adding the q previous items in row i 
                // printf("dataxik %d\n", data->X[starting_index+i][k]);
            }
            
            // get the average of the sum of the portion of the row
            double ma_estimate = current_ma_sum / q;

        
            // Xij-MAi(q) part of the formula
            double error = (double)data->X[starting_index+i][j] - ma_estimate;
            // summation part of first formula
            squared_errors_sum += (error * error);
        }

        
        // pj = squareroot mse's then all over m-q
        v[i] = sqrt(squared_errors_sum )/ (n - q);
        // printf(" wow p[%d] = %f  \n",starting_index+i, v[i]); //verify vi value and index
        data->r[starting_index + i] = v[i];    // This is for double *r
        // printf("hotdogdatar[%d]=%fhotdog\n", starting_index + i,data->r[starting_index + i]);//PRINT TO CHECKDATARINDEX AND WHAT TO ASSIGN IT TO (RIGHT V[I] VALUE)
        

    }
free(v);
    // return p;free(v);



}

int main() {
  int n, i, j, rows, columns, q, t, remainder, finalrows, numberofmatrices;
  float actualnovert;
struct timeval t1, t2;
    double elapsedTime;
  // (1) Read n as a user input (maybe from a command line or as a data stream);
//   printf("Enter n and t: ");
//   scanf("%d %d", &n, &t);
  printf("Enter n: ");
  scanf("%d", &n);
  printf("Enter t: ");
  scanf("%d", &t);
  printf("\nn:%d\n", n);
  printf("t:%d\n", t);

  numberofmatrices = t;
  rows = n;
  columns = n;
//   actualnovert = (float)n/t;
//   finalrows = n/t;
//   printf("n/t or number of rows each submatrix:%f\n", actualnovert);
//   remainder =  n%t;
//   printf("n/t remainder:%d\nn/t final number of rows %d\nnumber of matrices or t: %d\n", remainder, finalrows, numberofmatrices);
  //   (2) Create a non-zero n  n square matrix X whose elements are assigned with random integers
  // (make sure that any integer i  0);
  //Dynamically allocate memory for the array of pointers (rows)


//   // dynamically allocate vector por of size n x 1
//     double *por = (double *)malloc(m * sizeof(double));
//     if (por == NULL) return NULL;



    int **arr = (int **)malloc(rows * sizeof(int *));
    if (arr == NULL) {
        perror("malloc failed for rows");
        return 1;
    }
        //Dynamically allocate memory for each row (columns)
    for (int i = 0; i < rows; i++) {
        arr[i] = (int *)malloc(columns * sizeof(int));
        if (arr[i] == NULL) {
            perror("malloc failed for columns");
            // Free previously allocated memory before exiting
            for (int j = 0; j < i; j++) free(arr[j]);
            free(arr);
            return 1;
        }
    }
    //random nums range from 1 to 100
    int min = 1;
    int max = 100;
    //filling array with random numbers from range
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
          arr[i][j] = (rand() % (max - min + 1)) + min;
      }
  }


  // print array
    printf("array X of size %d x %d:\n", rows, columns);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%4d", arr[i][j]);
        }
        printf("\n");
    }
///////////////////////////////////////
// (3) Divide your X into t submatrices of size n x n/t each, which we will respectively call as the
// submatrices x1, x2, ..., xt;

    struct ThreadArg *args = (struct ThreadArg *)malloc(t * sizeof(struct ThreadArg));
    pthread_t *threads = (pthread_t *)malloc(t * sizeof(pthread_t));

    if (args == NULL || threads == NULL) {
        perror("Failed to allocate threads/args");
        return 1;
    }
////////////////////////////////////
    // printf("Process starting...\n");


    // printf("Dimensions of each submatrix:\n");
    // for(int i = 0; i < t; i++){
    //     printf("Submatrix %d: (%d x %d)\n", i, n, n/t + (i < n % t ? 1 : 0));
    // }


  // (3) Personalize q = max(n/2, min((10S1+S2)  n/100, 3n/4)), where S1 and S2 are the first and the
  // second items, respectively, of a sequence created by sorting in non-increasing order the
  // digits of your 5-digit student number; 76300 202067003 s1=7 s2=6
    // q=max(n/2, min((10*7+6)*n/100,3*n/4));
    q=(n/2 > (((10*7+6)*n/100 < 3*n/4?(10*7+6)*n/100: 3*n/4)))? n/2 : (((10*7+6)*n/100 < 3*n/4?(10*7+6)*n/100: 3*n/4));
    // printf("q= %d\n", q);

  // (4) Create a n  1 vector p;
  // int **arr = (int **)malloc(rows * sizeof(int *));
    double *p = (double *)malloc(n * sizeof(double));
    if (p == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    // printf("afterp\n");
    




    time_t time_before, time_after;
    
    double time_elapsed;

    // (4) Take note of the system time time_before;
    time(&time_before);
    clock_t start = clock();
    gettimeofday(&t1, NULL);
    // printf()
//     (5)Create t threads, where in the ith thread call mse_ma(xi, n, n/t) for all 1 ≤ i ≤ t;← very
// important
    int starting_index = 0;
    for (int i=0; i<t; i++){
         // Assign the submatrix to the current thread
        args[i].X = arr;
        // args[i].y = &p;                             // Point to the beginning of the vector y
        args[i].m = (n / t + (i < n % t ? 1 : 0)); // Total number of rows in each submatrix
        args[i].starting_index = starting_index;   // tracks the starting index of the submatrix in X
        args[i].n = n;                             // Number of columns in the submatrix
        args[i].r = p;                             // Point to the beginning of the result vector r
        args[i].index = i;
        args[i].q = q;                         // Index of the current thread

        // // Create a thread and pass the thread function mse_ma2 along with the thread arguments
        pthread_create(&threads[i], NULL, mse_ma2, &args[i]);
        starting_index += (n / t + (i < n % t ? 1 : 0));
    }
    // Join t threads
    for (int i = 0; i < t; i++)
        pthread_join(threads[i], NULL);
    // (6) create p via a call to mse_ma(X, q, n, n);
    // mse_ma(arr, q, n, n);
    // mse_ma(arr, q, n, n, p);

    // (7) Take note of the system time time_after;
    time(&time_after);
    clock_t end = clock();
    gettimeofday(&t2, NULL);
    // (8) Obtain the elapsed time time_elapsed:=time_after – time_before;
    // time_elapsed = difftime(time_after, time_before);
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
elapsedTime = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    printf("Time elapsed: %lf seconds\n", elapsedTime);
    // printf("Task ended.\n");
    // (9) output time_elapsed;
    // printf("time elapsed: %.4lf seconds\n", time_elapsed);
    // printf("\ntime elapsed: %.4lf seconds\n", cpu_time_used);

// PRINTING P VECTOR IN MAIN
// for (i=0;i<n;i++){
//     printf("p[%d]= %f \n", i, p[i]);
// }



// PUHLEASE


      // Free the dynamically allocated memory
    for (int i = 0; i < rows; i++) {
        free(arr[i]);
        
    }
    free(arr);
    free(p);
    
  return 0;
}


