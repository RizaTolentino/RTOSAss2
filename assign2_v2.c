/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.

//***********************************************************************************
/***********************************************************************************/

/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc prog_1.c -o prog_1 -lpthread -lrt

*/
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/time.h>
#include <stdbool.h>

/* --- Defines --- */
//#define CONTENT_START 9
#define TRUE 1
#define FALSE 0
/* --- Structs --- */

typedef struct ThreadParams
{
  int pipeFile[2];
  sem_t sem_read, sem_justify, sem_write;
  char message[255];
} ThreadParams;

/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void *ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void *ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void *ThreadC(void *params);

/* --- Main Code --- */
int main(int argc, char const *argv[])
{

  int result;
  pthread_t tid1, tid2, tid3; //Thread ID
  pthread_attr_t attr;        //Thread Attributes
  ThreadParams params;        //Struct to store pipes, message and semaphores. To be passed into threads.

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);

  // Create Threads
  if (pthread_create(&tid1, &attr, ThreadA, (void *)(&params)) != 0)
  {
    //If unsuccessful, exit
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid2, &attr, ThreadB, (void *)(&params)) != 0)
  {
    //If unsuccessful, exit
    perror("Error creating threads: ");
    exit(-1);
  }
  if (pthread_create(&tid3, &attr, ThreadC, (void *)(&params)) != 0)
  {
    //If unsuccessful, exit
    perror("Error creating threads: ");
    exit(-1);
  }
  
  printf("Output file created, end program\n");

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);

}

void initializeData(ThreadParams *params)
{
  // Create pipe
  if (pipe(params->pipeFile) < 0)
  {
    //If unsuccessful, exit
    perror("pipe error");
    exit(1);
  }
  // Initialize Sempahores in the thread respective thread

}

void *ThreadA(void *params)
{
  //Declare local variables
  ThreadParams *threadA_Params = (ThreadParams*)(params); //Pointer to thread struct
	FILE* fp;                                               //File pointer to txt file to be read
	char str[512];                                          //String to store text from file

  //Initialise semaphore
  if(!sem_init(&(threadA_Params->sem_write), 0, 1))
    printf("Successfully initialised Sem_A\n");
  else
  {
    //If semaphore is not initalised correctly, exit porgram
    perror("Semaphore not initialised correctly:");
    exit(1);
  }

  //Open data text file
	fp = fopen("data.txt", "r");

  if (fp == NULL)
  {
    //If semaphore is not initalised correctly, exit porgram
    perror("Error opening file:");
    exit(1);
  }

  // //put text file into string
  // if(fread(str, sizeof(str)+1, 1,fp) < 0)
  // {
  //   perror("Error reading file");
  //   exit(1);
  // }
  // printf("This is string: %s", str);

  //close file stream    
  

  //cut string at newline character
  char strline[255];
	while(1)
	{
    //The thread must wait for sempahore to be posted
		sem_wait(&(threadA_Params->sem_write));
		
    fgets(strline, 256, fp);

		if(feof(fp))
    {
      fclose(fp);
      //printf("Last line of file\n");
      exit(0);
    }

      
		printf("thread one: write to pipe\n");
		printf("writing: %s \n", strline);

		write(threadA_Params->pipeFile[1], strline, 80);

		// strline = NULL;
		// strline = strtok(NULL, "\n");
		// strline = strtok(NULL, "\n");

    printf("Post read semaphore\n\n");
		sem_post(&(threadA_Params->sem_read));
	}
  //change
  printf("ThreadA\n");
}

void *ThreadB(void *params)
{
  ThreadParams *threadB_Params = (ThreadParams*)(params);

  if(!sem_init(&(threadB_Params->sem_read), 0, 0))
    printf("Successfully initialised Sem_B\n");
  /*read from pipe*/
  while(1){
    
   int     i;  
   sem_wait(&(threadB_Params->sem_read));
    printf ("In reading thread\n");
    read (threadB_Params->pipeFile[0],threadB_Params->message,256);
    
    printf("read from pipe: ");
    for (i = 0; i < strlen(threadB_Params->message); i++){
      printf ("%c", threadB_Params->message[i]);}

    printf("\nreading pipe has completed\n");
    
    //currently message[256] stores a line of data in a 1D array
    printf("posting Thread C semaphore\n\n");
    sem_post(&(threadB_Params->sem_justify));
  }
  
  printf("ThreadB\n");
}

void *ThreadC(void *params)
{
  ThreadParams *threadC_Params = (ThreadParams*)(params);
  //TODO: add your code
  bool ContentFlag = FALSE;
  FILE * fp;
  if(!sem_init(&(threadC_Params->sem_justify), 0, 0))
    printf("Successfully initialised Sem_C\n");

  remove("output.txt");

  while(1)
  {
    sem_wait(&(threadC_Params->sem_justify));
    printf("in Thread C\n");
    if (threadC_Params->message == NULL)
      exit(5);
    char *ch = strcat((threadC_Params->message),"\n");

    if (ContentFlag == TRUE)
    {
      printf("%s", ch);
      //open .txt file, append to file
      fp = fopen("output.txt","a");
      //print to txt file
      fprintf(fp, "%s", ch);
      //close txt file
      fclose(fp);
    }

    //Check if we are in the content region
    if (!strcmp(ch,"end_header\n"))
    {
      printf("found end of header\n");
      //it is now content stuff
      ContentFlag = TRUE;
    }
      
    //post write sempahore
    printf("Post Write Semaphore\n\n");
    sem_post(&(threadC_Params->sem_write));
    
  }
  
  printf("ThreadC\n");
}
