#include <iostream>
#include <chrono>
#include <ratio>
#include <ctime>
#include <omp.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
#endif

    void generatePrefixSumData (int* arr, size_t n);
    void checkPrefixSumResult (int* arr, size_t n);


#ifdef __cplusplus
}
#endif


int main (int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
        return -1;
    }
    //forces openmp to create the threads beforehand
    
#pragma omp parallel
    {
        int fd = open (argv[0], O_RDONLY);
        if (fd != -1) {
            close (fd);
        }
        else {
            std::cerr<<"something is amiss"<<std::endl;
        }
    }

    int i, n = atoi(argv[1]);

    int * arr = new int [n];

    int * prefixArray = new int [n+1];
    int numOfThreads = atoi(argv[2]);

   int *temp ;

    prefixArray[0]=0; //setting the first element of prefixArray to zero

    omp_set_dynamic(0);  //disabling dynamic adjustment of threads
    omp_set_num_threads(numOfThreads);//setting number of threads based on cooman line input

#pragma omp_set_schedule(omp_sched_static, chunkSize);  //defaut directive to make a static schedule

    generatePrefixSumData (arr, n);  //getting the array data

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();//starting clock
#pragma omp parallel
    {
        int id = omp_get_thread_num(); 
        int sum = 0;

#pragma omp single     //single thread computation , will be excuted only once 
        {
            numOfThreads = omp_get_num_threads();
            temp = new int[numOfThreads];   //setting the size of temp array to numOfThreads available
        }
#pragma omp for schedule(static)  
        for ( i = 0; i<n; i++ )
        {
            sum += arr[i];
            prefixArray[i+1] = sum;
        }
#pragma omp critical   	//single thread computation
        temp[id] = sum;
#pragma omp barrier    //all threads will wait here for computations to complete
        int offset = 0;
        for (i=0; i<id; i++)
        {
            offset += temp[i]; 
        }
#pragma omp for schedule(static)   //keep adding the offset values to the next index of prefixArray
        for (i=0;i< n;i++ )
        {
            prefixArray[i+1] += offset;
        }
    }
    std::chrono::high_resolution_clock::time_point end  = std::chrono::high_resolution_clock::now();//stopping clock

    std::chrono::duration<double> elapsed_seconds = end-start;   //calculating the elapsed time in  seconds

    checkPrefixSumResult(prefixArray,n);  //to check the results

    std::cerr<<std::fixed<<elapsed_seconds.count()<<std::endl;  //displaying time to error stream 

    //releasing memory
    delete[] temp;
    delete[] arr;
    delete[] prefixArray;


    return 0;
}

