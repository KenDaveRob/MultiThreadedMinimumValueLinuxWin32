#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define NAME "Kenneth Robertson"

typedef struct 
{
	int startIndex;
	int endIndex;
	int threadNumber;
}ThreadInput;

int *dataArray;
int *minIndices;
BOOL *validRanges;
int threadNumber;
int arrayPower;
int threadPower;

DWORD WINAPI threadInsertSort(LPVOID lpParam);
void insertionSort();

//Function generates the integer 2^power using bitwise shifting
int binaryPower(int power);
int get_next();

int main(int argc, char *argv[])
{
	int i = 0;
	int arrayLength;
	HANDLE *threadList;
	LARGE_INTEGER frequency, start, stop;
	ThreadInput *threadInfos;
	int *dataArrayCopy;

	arrayPower = atoi(argv[1]);
	threadPower = atoi(argv[2]);

	arrayLength = binaryPower(arrayPower);
	threadNumber = binaryPower(threadPower);
	threadList = (HANDLE*) malloc(sizeof(HANDLE)*threadNumber);
	dataArray = (int*) malloc(sizeof(int) * arrayLength);
	dataArrayCopy = (int*) malloc(sizeof(int) * arrayLength);
	minIndices = (int*) malloc(sizeof(int) * threadNumber);
	threadInfos = (ThreadInput*) malloc(sizeof(ThreadInput) * threadNumber);
	QueryPerformanceFrequency(&frequency);
	srand(3);

	validRanges = (BOOL*) malloc(sizeof(BOOL) * threadNumber);
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

		
	QueryPerformanceCounter(&start);
	for(i = 0; i < threadNumber; i++)
	{
		threadInfos[i].startIndex = i*binaryPower(arrayPower-threadPower);
		threadInfos[i].endIndex = (i+1)*binaryPower(arrayPower-threadPower);
		threadInfos[i].threadNumber = i;		
			
		threadList[i] = CreateThread(NULL, 0, threadInsertSort, &threadInfos[i], 0, NULL);
		if(threadList[i] == NULL)
		{
			fprintf(stderr, "Error: CreateThread, thread %i, index %i.\n",threadList[i], i);
			ExitProcess(i);
		}
	}

	if(WAIT_FAILED == WaitForMultipleObjects(threadNumber, threadList, TRUE, INFINITE))
		fprintf(stderr,"ERROR: WaitForMultipleObjects returned WAIT_FAILED\n");
	QueryPerformanceCounter(&stop);


	if(argc < 4 || (argv[3][0] != 'd' && argv[3][0] != '-')) //use '-' or 'd' as third arugment if you dont want to list array
	{
		printf("\n\nSORTED IN PARALLEL\n\n");
		for(i = 0; i < arrayLength; i++)
			printf("Position %i = %i\n",i,get_next());
	}

	printf("\nIt took %lf microseconds for %i threads to be created and complete there searches.\n", ((double)(stop.QuadPart - start.QuadPart)*1000000.0)/((double)frequency.QuadPart), threadNumber);

	free(dataArray);
	dataArray = dataArrayCopy;

	QueryPerformanceCounter(&start);
	insertionSort();
	QueryPerformanceCounter(&stop);

	printf("\nIt took %lf microseconds to perform the insertion sort using the main thread.\n", ((double)(stop.QuadPart - start.QuadPart)*1000000.0)/((double)frequency.QuadPart), threadNumber);	

	free(validRanges);
	free(minIndices);
	free(dataArrayCopy);
	free(threadList);
	return 0;
}

DWORD WINAPI threadInsertSort( LPVOID lpParam ) 
{
	ThreadInput *threadInfo = ((ThreadInput*)lpParam);
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

	return 1;
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
		if (dataArray[minIndices[i]] < min && validRanges[i]) 
		{
			min_index = i;
			min = dataArray[minIndices[min_index]];
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
Compiled using Microsoft Visual Studio 2010

I added an extra command line to turn off the printing of the array, for really big arrays.  Thats why only the first two outputs print the contents.

It took 504934273 ns for 8 threads to sort 65536 integers, this is 504934.273 microseconds for the linux version.  
The windows version took 114240.944589 microseconds (about a 4.5 times slower on linux).
It took 1 s, 89226404 ns for 4 threads to sort 65536 integers, this is 1089226.404 microseconds for the linux version.  
The windows version took 316061.256549 microseconds (about 3.5 times slower on linux).
*/

//Output
/*
C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 4 1
Initial Array

dataArray[0] = 48
dataArray[1] = 7196
dataArray[2] = 9294
dataArray[3] = 9091
dataArray[4] = 7031
dataArray[5] = 23577
dataArray[6] = 17702
dataArray[7] = 23503
dataArray[8] = 27217
dataArray[9] = 12168
dataArray[10] = 5409
dataArray[11] = 28233
dataArray[12] = 2023
dataArray[13] = 17152
dataArray[14] = 21578
dataArray[15] = 2399


SORTED IN PARALLEL

Position 0 = 48
Position 1 = 2023
Position 2 = 2399
Position 3 = 5409
Position 4 = 7031
Position 5 = 7196
Position 6 = 9091
Position 7 = 9294
Position 8 = 12168
Position 9 = 17152
Position 10 = 17702
Position 11 = 21578
Position 12 = 23503
Position 13 = 23577
Position 14 = 27217
Position 15 = 28233

It took 292.965611 microseconds for 2 threads to be created and complete there searches.

It took 0.853298 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 5 2
Initial Array

dataArray[0] = 48
dataArray[1] = 7196
dataArray[2] = 9294
dataArray[3] = 9091
dataArray[4] = 7031
dataArray[5] = 23577
dataArray[6] = 17702
dataArray[7] = 23503
dataArray[8] = 27217
dataArray[9] = 12168
dataArray[10] = 5409
dataArray[11] = 28233
dataArray[12] = 2023
dataArray[13] = 17152
dataArray[14] = 21578
dataArray[15] = 2399
dataArray[16] = 23863
dataArray[17] = 16025
dataArray[18] = 8489
dataArray[19] = 19718
dataArray[20] = 22454
dataArray[21] = 12798
dataArray[22] = 1164
dataArray[23] = 14182
dataArray[24] = 29498
dataArray[25] = 1731
dataArray[26] = 27271
dataArray[27] = 18899
dataArray[28] = 6936
dataArray[29] = 27897
dataArray[30] = 11449
dataArray[31] = 31232


SORTED IN PARALLEL

Position 0 = 48
Position 1 = 1164
Position 2 = 1731
Position 3 = 2023
Position 4 = 2399
Position 5 = 5409
Position 6 = 6936
Position 7 = 7031
Position 8 = 7196
Position 9 = 8489
Position 10 = 9091
Position 11 = 9294
Position 12 = 11449
Position 13 = 12168
Position 14 = 12798
Position 15 = 14182
Position 16 = 16025
Position 17 = 17152
Position 18 = 17702
Position 19 = 18899
Position 20 = 19718
Position 21 = 21578
Position 22 = 22454
Position 23 = 23503
Position 24 = 23577
Position 25 = 23863
Position 26 = 27217
Position 27 = 27271
Position 28 = 27897
Position 29 = 28233
Position 30 = 29498
Position 31 = 31232

It took 308.324973 microseconds for 4 threads to be created and complete there searches.

It took 2.559894 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 8 1 -

It took 197.111814 microseconds for 2 threads to be created and complete there searches.

It took 69.685995 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 8 2 -

It took 229.252702 microseconds for 4 threads to be created and complete there searches.

It took 70.254860 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 8 3 -

It took 374.597777 microseconds for 8 threads to be created and complete there searches.

It took 71.677023 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 16 1 -

It took 1006571.531536 microseconds for 2 threads to be created and complete there searches.

It took 3983588.237118 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 16 2 -

It took 316061.256549 microseconds for 4 threads to be created and complete there searches.

It took 3992256.321586 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 16 3 -

It took 114240.944589 microseconds for 8 threads to be created and complete there searches.

It took 4115843.438040 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>OSHW8.exe 20 3 -

It took 28737256.778101 microseconds for 8 threads to be created and complete there searches.

It took 1021305312.547376 microseconds to perform the insertion sort using the main thread.

C:\Users\Kenneth\Documents\Visual Studio 2010\Projects\OSHW8\Debug>

*/