#include <stdio.h> 
#include <pthread.h> 
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>



#define NAME "Kenneth Robertson"

/*
COMPILED USING TERMINAL COMMAND:
cc OSHW8.c -o OSHW8 -pthread -lrt
*/


typedef struct 
{
	int startIndex;
	int endIndex;
	int threadNumber;
}ThreadInput;

int *dataArray;
int *minIndices;
bool *validRanges;
int threadNumber;
int arrayPower;
int threadPower;

void *threadInsertSort(void *param);
void insertionSort();

//Function generates the integer 2^power using bitwise shifting
int binaryPower(int power);

struct timespec timeDifference(struct timespec start, struct timespec end);

int get_next();

int main(int argc, char **argv)
{	
	int i = 0;
	int arrayLength;
	int returnValue = 0;
	pthread_t *threadList;
	struct timespec start;
	struct timespec end;
	struct timespec diff;
	ThreadInput *threadInfos;
	int *dataArrayCopy;
	pthread_attr_t attr;

	arrayPower = atoi(argv[1]);
	threadPower = atoi(argv[2]);

	arrayLength = binaryPower(arrayPower);
	threadNumber = binaryPower(threadPower);
	threadList = (pthread_t*) malloc(sizeof(pthread_t)*threadNumber);
	dataArray = (int*) malloc(sizeof(int) * arrayLength);
	dataArrayCopy = (int*) malloc(sizeof(int) * arrayLength);
	minIndices = (int*) malloc(sizeof(int) * threadNumber);
	threadInfos = (ThreadInput*) malloc(sizeof(ThreadInput) * threadNumber);

	srand(3);

	validRanges = (bool*) malloc(sizeof(bool) * threadNumber);
	for(i = 0; i < threadNumber; i++)
		validRanges[i] = 1;

	for(i = 0; i < arrayLength; i++)
	{
		dataArray[i] = rand();
		dataArrayCopy[i] = dataArray[i];
	}

	
	if(argc < 4 || (argv[3][0] != 'd' && argv[3][0] != '-')) //use '-' or 'd' as third arugment if you dont want to list array
	{
		printf("Initial Array\n\n");
		for(i = 0; i < arrayLength; i++)
			printf("dataArray[%i] = %i\n",i,dataArray[i]);
	}

	
	srand(3);//use (unsigned)time(NULL) to get new set each time

	returnValue = pthread_attr_init(&attr);
	
	if(returnValue)
	{
		fprintf(stderr, "Error: pthread_attr_init failed, returning %i\n", returnValue);
		exit(-1);
	}
	

	

	clock_gettime(CLOCK_REALTIME, &start);
	
	for(i = 0; i < threadNumber; i++)
	{
		threadInfos[i].startIndex = i*binaryPower(arrayPower-threadPower);
		threadInfos[i].endIndex = (i+1)*binaryPower(arrayPower-threadPower);
		threadInfos[i].threadNumber = i;	

		returnValue = pthread_create(&threadList[i], &attr, threadInsertSort, &threadInfos[i]);  
		if(returnValue)
		{
			fprintf(stderr, "Error: pthread_create, thread %i, returning %i\n",i, returnValue);
			exit(-1);
		}
	}


	for(i = 0; i < threadNumber; i++)
	{
		returnValue = pthread_join(threadList[i], NULL); 
		if(returnValue)
		{
			fprintf(stderr, "Error: pthread_join thread %i\n, returning %i",i, returnValue);
			exit(-1);
		}
	}
	clock_gettime(CLOCK_REALTIME, &end);
	
	if(argc < 4 || (argv[3][0] != 'd' && argv[3][0] != '-')) //use '-' or 'd' as third arugment if you dont want to list array
	{
		printf("\n\nSORTED IN PARALLEL\n\n");
		for(i = 0; i < arrayLength; i++) 
			printf("Position %i = %i\n",i,get_next());
	}
	
	diff = timeDifference(start, end);
	printf("\nIt took %li s, %li ns for %i threads to be created and complete their searches.\n",diff.tv_sec, diff.tv_nsec, threadNumber);

	free(dataArray);
	dataArray = dataArrayCopy;

	clock_gettime(CLOCK_REALTIME, &start);
	insertionSort();
	clock_gettime(CLOCK_REALTIME, &end);

	diff = timeDifference(start, end);
	printf("\nIt took %li s, %li ns to perform the insertion sort using the main thread.\n",diff.tv_sec, diff.tv_nsec);

	free(validRanges);
	free(minIndices);
	free(dataArrayCopy);
	free(threadList);
	

	return 0;
}

void *threadInsertSort(void *param)
{
	ThreadInput *threadInfo = ((ThreadInput*)param);
	int i;
	int temp;
	int currentIndex;
	
	
	for(i = threadInfo->startIndex; i < threadInfo->endIndex; i++) 
	{
		currentIndex = i;
 
		while (currentIndex > threadInfo->startIndex && dataArray[currentIndex] < dataArray[currentIndex-1]) 
		{
			  temp = dataArray[currentIndex];
			  dataArray[currentIndex] = dataArray[currentIndex-1];
			  dataArray[currentIndex-1] = temp;
 
			  currentIndex--;
		}
	}

	minIndices[threadInfo->threadNumber] = threadInfo->startIndex; 
	

	pthread_exit(0);	
}

struct timespec timeDifference(struct timespec start, struct timespec end)
{
	struct timespec diff;

	diff.tv_sec = end.tv_sec - start.tv_sec;
	diff.tv_nsec = end.tv_nsec - start.tv_nsec;

	if(diff.tv_nsec < 0)
	{
		diff.tv_sec--;
		diff.tv_nsec += 1000000000;
	}

	return diff;
}

void insertionSort()
{
	int i;
	int temp;
	int currentIndex;

	for(i = 1; i < binaryPower(arrayPower); i++) 
	{
		currentIndex = i;
 
		while (currentIndex > 0 && dataArray[currentIndex] < dataArray[currentIndex-1]) 
		{
			  temp = dataArray[currentIndex];
			  dataArray[currentIndex] = dataArray[currentIndex-1];
			  dataArray[currentIndex-1] = temp;
 
			  currentIndex--;
		}
	}
}

//I couldn't get the code given in assignment to work as it was.  
//The problem was that when a partition was completely taken from there was no logic stoping get_next from continuing on into the next partition.
//To fix this problem I created an array of BOOLs to keep track of which partitions have been exhausted, 
//I allso added code so that the intial min_inidex is not one of these exhausted partitions
int get_next() 
{
	int i = 1;
	int min_index = 0;
	int min;
	while(minIndices[min_index] >= (min_index+1)*binaryPower(arrayPower-threadPower))
		min_index++;
	
	min = dataArray[minIndices[min_index]];

	for (i = 1; i < threadNumber; i++)
	{
		if (dataArray[minIndices[i]] < min && validRanges[i] != 0) 
		{
			min_index = i;
			min = dataArray[minIndices[min_index]];
		}
	}


	minIndices[min_index]++;

	if(minIndices[min_index] >= (min_index+1)*binaryPower(arrayPower-threadPower)) validRanges[min_index] = 0;

	return min;
}

int binaryPower(int power)
{
	return (2 << (power-1));
}

//Additional Information
/*
Results were produced on a Intel i7-3820 @ 3.60 ghz;  This has 4 actual cores each with 2 threads.  
Compiled using gcc on ubuntu 12.04.3 i386 VirtualBox using console command: cc OSHW8.c -o OSHW8 -pthread -lrt

I added an extra command line to turn off the printing of the array, for really big arrays.  Thats why only the first two outputs print the contents.

It took 504934273 ns for 8 threads to sort 65536 integers, this is 504934.273 microseconds for the linux version.  
The windows version took 114240.944589 microseconds (about a 4.5 times slower on linux).
It took 1 s, 89226404 ns for 4 threads to sort 65536 integers, this is 1089226.404 microseconds for the linux version.  
The windows version took 316061.256549 microseconds (about 3.5 times slower on linux).
*/


//OUTPUT
/*
adminuser@adminuser-VirtualBox:~$ ./OSHW8 4 1
Initial Array

dataArray[0] = 1205554746
dataArray[1] = 483147985
dataArray[2] = 844158168
dataArray[3] = 953350440
dataArray[4] = 612121425
dataArray[5] = 310914940
dataArray[6] = 1210224072
dataArray[7] = 1856883376
dataArray[8] = 1922860801
dataArray[9] = 495649264
dataArray[10] = 8614858
dataArray[11] = 989089924
dataArray[12] = 378651393
dataArray[13] = 1344681739
dataArray[14] = 2029100602
dataArray[15] = 1816952841


SORTED IN PARALLEL

Position 0 = 8614858
Position 1 = 310914940
Position 2 = 378651393
Position 3 = 483147985
Position 4 = 495649264
Position 5 = 612121425
Position 6 = 844158168
Position 7 = 953350440
Position 8 = 989089924
Position 9 = 1205554746
Position 10 = 1210224072
Position 11 = 1344681739
Position 12 = 1816952841
Position 13 = 1856883376
Position 14 = 1922860801
Position 15 = 2029100602

It took 0 s, 173884 ns for 2 threads to be created and complete their searches.

It took 0 s, 921 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 5 2
Initial Array

dataArray[0] = 1205554746
dataArray[1] = 483147985
dataArray[2] = 844158168
dataArray[3] = 953350440
dataArray[4] = 612121425
dataArray[5] = 310914940
dataArray[6] = 1210224072
dataArray[7] = 1856883376
dataArray[8] = 1922860801
dataArray[9] = 495649264
dataArray[10] = 8614858
dataArray[11] = 989089924
dataArray[12] = 378651393
dataArray[13] = 1344681739
dataArray[14] = 2029100602
dataArray[15] = 1816952841
dataArray[16] = 21468264
dataArray[17] = 552076975
dataArray[18] = 87517201
dataArray[19] = 953369895
dataArray[20] = 374612515
dataArray[21] = 787097142
dataArray[22] = 126313438
dataArray[23] = 1207815258
dataArray[24] = 287632273
dataArray[25] = 1886964647
dataArray[26] = 1220723885
dataArray[27] = 1119448937
dataArray[28] = 444268468
dataArray[29] = 1865680798
dataArray[30] = 1654563454
dataArray[31] = 1649823214


SORTED IN PARALLEL

Position 0 = 8614858
Position 1 = 21468264
Position 2 = 87517201
Position 3 = 126313438
Position 4 = 287632273
Position 5 = 310914940
Position 6 = 374612515
Position 7 = 378651393
Position 8 = 444268468
Position 9 = 483147985
Position 10 = 495649264
Position 11 = 552076975
Position 12 = 612121425
Position 13 = 787097142
Position 14 = 844158168
Position 15 = 953350440
Position 16 = 953369895
Position 17 = 989089924
Position 18 = 1119448937
Position 19 = 1205554746
Position 20 = 1207815258
Position 21 = 1210224072
Position 22 = 1220723885
Position 23 = 1344681739
Position 24 = 1649823214
Position 25 = 1654563454
Position 26 = 1816952841
Position 27 = 1856883376
Position 28 = 1865680798
Position 29 = 1886964647
Position 30 = 1922860801
Position 31 = 2029100602

It took 0 s, 235003 ns for 4 threads to be created and complete their searches.

It took 0 s, 1702 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 8 1 -

It took 0 s, 262249 ns for 2 threads to be created and complete their searches.

It took 0 s, 84769 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 8 2 -

It took 0 s, 299739 ns for 4 threads to be created and complete their searches.

It took 0 s, 92517 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 8 3 -

It took 0 s, 363619 ns for 8 threads to be created and complete their searches.

It took 0 s, 83620 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 16 1 -

It took 2 s, 122910975 ns for 2 threads to be created and complete their searches.

It took 7 s, 479273883 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 16 2 -

It took 1 s, 89226404 ns for 4 threads to be created and complete their searches.

It took 7 s, 471482994 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ ./OSHW8 16 3 -

It took 0 s, 504934273 ns for 8 threads to be created and complete their searches.

It took 7 s, 606902426 ns to perform the insertion sort using the main thread.
adminuser@adminuser-VirtualBox:~$ 

*/

