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
#define TRUE 1
#define FALSE 0
#define MAX_STR_LENGTH 255
#define NB_PIPES 2

/* --- Structs --- */

typedef struct ThreadParams
{
  int pipeFile[NB_PIPES];                   //pipes for the threads to communicate
  sem_t sem_A_to_B, sem_B_to_C, sem_C_to_A; //semaphore to protect sections
  char message[MAX_STR_LENGTH];             //Store message for IPC
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

  //Initialise C_to_A semaphore
  if(!sem_init(&(params->sem_C_to_A), 0, 1))
    printf("Successfully initialised Sem_C_to_A\n");

  else
  {
    //If semaphore is not initalised correctly, exit program
    perror("Semaphore Sem_C_to_A not initialised correctly:");
    exit(1);
  }

  //Initialise A_to_B semaphore
  if(!sem_init(&(params->sem_A_to_B), 0, 0))
    printf("Successfully initialised Sem_A_to_B\n");

  else
  {
    //If semaphore is not initialised correctly, exit program
    perror("Sem_A_to_B not initialised correctly\n");
    exit(1);
  }

  //Initialise semaphore
  if(!sem_init(&(params->sem_B_to_C), 0, 0))
    printf("Successfully initialised Sem_B_to_C\n");

  else
  {
    //If semaphore is not initialised correctly, exit program
    perror("Sem_B_to_C not initialised correctly\n");
    exit(1);
  }

}

void *ThreadA(void *params)
{
  //Declare local variables
  ThreadParams *threadA_Params = (ThreadParams*)(params); //Pointer to thread struct
	FILE* fp;                                               //File pointer to txt file to be read
	char strline[MAX_STR_LENGTH];                           //String to store text from file

  //Open data text file
	fp = fopen("data.txt", "r");

  //If file is not opened correctly, exit porgram
  if (fp == NULL)
  {
    perror("Error opening file:");
    exit(1);
  }

	while(1)
	{
    //The thread must wait for sempahore to be posted
		sem_wait(&(threadA_Params->sem_C_to_A));
		
    //Detects end-of-file and closes the file
    
    if (fgets(strline, MAX_STR_LENGTH, fp) == NULL)
    {
      //there has been an error
      if (feof(fp) != 0)
      {
        printf("END OF FILE\n");
        fclose(fp);
        //successful exit
        exit(0);
      }
      else
      {
        perror("fgets has encountered an error");
        exit(1);
      }
    }

		printf("In Thread A: write to pipe\n");
		printf("writing: %s", strline);
    
    //if write is unsuccessful, exit program
		if (write(threadA_Params->pipeFile[1], strline, MAX_STR_LENGTH) < 0)
    {
      perror("Error writing to pipe\n");
      exit(1);
    }

    printf("Post semaphore - sem_A_to_B\n\n");
    //unlock read semaphore - signal Thread B
		sem_post(&(threadA_Params->sem_A_to_B));
	}

}

void *ThreadB(void *params)
{
  //Declare local variables
  ThreadParams *threadB_Params = (ThreadParams*)(params);

  //Read from pipe
  while(1)
  { 
    int i;  
    sem_wait (&(threadB_Params->sem_A_to_B));
    printf ("In thread B: reading pipe\n");
    
    //If reading from pipe is unsuccessful, exit program
    if (read (threadB_Params->pipeFile[0],threadB_Params->message,256) < 0)
    {
      perror ("Error reading from pipe");
      exit (1);
    }
    
    printf("read from pipe: ");

    //Store each character from pipe into message array
    for (i = 0; i < strlen(threadB_Params->message); i++){
      printf ("%c", threadB_Params->message[i]);}
    
    printf("Posting semaphore - sem_B_to_C\n\n");

    //unlock justify semaphore - signal Thread C
    sem_post(&(threadB_Params->sem_B_to_C));
  }

}

void *ThreadC(void *params)
{
  //Declare local variables
  ThreadParams *threadC_Params = (ThreadParams*)(params);
  bool ContentFlag = FALSE;
  FILE * fp;

  //Delete existing output file
  remove("output.txt");

  while(1)
  {
    sem_wait(&(threadC_Params->sem_B_to_C));
    printf("In Thread C: write to file\n");
    //Check if message array is empty
    if (threadC_Params->message == NULL)
    {
      perror("Nothing to read from message");
      exit(1);
    }

    //Append newline character to message
    char *ch = strcat((threadC_Params->message),"\n");

    //Detects if the line of data in message is part of the header or content
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
    printf("Post semaphore - sem_C_to_A\n\n");
    sem_post(&(threadC_Params->sem_C_to_A));
    
  }

}