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
#define CONTENT_START 9
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
  struct timeval t1;
  gettimeofday(&t1, NULL); // Start Timer
  int fd[2];               //File descriptor for creating a pipe

  int result;
  pthread_t tid1, tid2, tid3; //Thread ID
  pthread_attr_t attr;
  ThreadParams params;

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);


  // Create Threads
  if (pthread_create(&tid1, &attr, ThreadA, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid2, &attr, ThreadB, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  if (pthread_create(&tid3, &attr, ThreadC, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  //TODO: add your code

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  //TODO: add your code

  return 0;
}

void initializeData(ThreadParams *params)
{
  // Create pipe
  if (pipe(params->pipeFile) < 0)
  {
    perror("pipe error");
    exit(1);
  }
  // Initialize Sempahores
  sem_init(&(params->sem_read), 0, 0);
  sem_init(&(params->sem_write), 0, 1);
  sem_init(&(params->sem_justify), 0, 0);

  //Get default attributes
  //TODO: add your code
  //Initialise
  //*params->message = NULL;

}

void *ThreadA(void *params)
{
  int success = 100;
	//printf("Made it to runnerOne\n");
	FILE* fp;
	char str[256];
	fp = fopen("data.txt", "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    //put text file into string
    fread(str, sizeof(str)+1, 1,fp);
        //printf("This is string: %s", str);
        
    fclose(fp);

	int *fd = (int*)(((ThreadParams*)(params))->pipeFile);
	/* aquire the mutex lock */
	
	char *strline = strtok(str, "\n");
	while(1)
	{
		// success = pthread_mutex_lock(&mutex);
		sem_wait(&((sem_t)(((ThreadParams*)(params))->sem_write)));
		
		if(strline == NULL)
			exit(1);
      
		printf("thread one: write to pipe\n");
		printf("writing: %s \n\n", strline);

		write(fd[1], strline, 80);
		// printf("success for lock is: %d\n", success);
		
		/* release the mutex lock */
		// pthread_mutex_unlock(&mutex);
		strline = NULL;
		strline = strtok(NULL, "\n");
		sem_post(&((sem_t)(((ThreadParams*)(params))->sem_read)));
	}
  //change
  printf("ThreadA\n");
}

void *ThreadB(void *params)
{
  printf ("In reading thread\n");
  int *pipeFile = (int*)(((ThreadParams*)(params))->pipeFile);
  char *message =  (char*)(((ThreadParams*)(params))->message);
  /*read from pipe*/
  while(1){
    //char    ch[256];
    int     i;  
    
    read (pipeFile[0],message,256);
    
    for (i = 0; i < strlen(message); i++){
      printf ("%c", message[i]);}
    printf("reading pipe has completed\n");
    
    /*store in 2D array*/
    //currently message[256] stores a line of data in a 1D array

    sem_post(&((sem_t)(((ThreadParams*)(params))->sem_justify)));
  }
  
  printf("ThreadB\n");
}

void *ThreadC(void *params)
{
  //TODO: add your code
  bool ContentFlag = FALSE;
  FILE * fp;
  for(;;)
  {
    sem_wait(&((sem_t)(((ThreadParams*)(params))->sem_justify)));
    char *ch = strcat((char*)(((ThreadParams*)(params))->message),"\n");

    if (ContentFlag = TRUE)
    {
      printf("%s", ch);
      //open .txt file, append to file
      fp = fopen("output.txt","a");
      
      //print to txt file
      fprintf(fp, ch);
      //close txt file
      
      fclose(fp);
    }

    if (strcmp(ch,"end_header\n"))
      //it is now content stuff
      ContentFlag = TRUE;

  
    //post write sempahore
    sem_post(&((sem_t)(((ThreadParams*)(params))->sem_write)));
  }
  

  printf("ThreadC\n");
}
