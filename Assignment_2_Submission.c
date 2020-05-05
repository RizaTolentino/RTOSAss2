/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.
// Authors: Rachel Nicoll 12876215 & Riza Tolentino 12876254
//***********************************************************************************
/***********************************************************************************/

/*
  To compile Assignment_2_Submission ensure that gcc is installed and run the following command:
  gcc Assignment_2_Submission.c -o Assignment_2_Submission.o -lpthread -lrt -O2

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
  char inputFilename[MAX_STR_LENGTH];       //name of file to read
  char outputFilename[MAX_STR_LENGTH];      //name of file to write
  int pipeFile[NB_PIPES];                   //pipes for the threads to communicate
  sem_t sem_A_to_B, sem_B_to_C, sem_C_to_A; //semaphore to protect sections
  char message[MAX_STR_LENGTH];             //Store message for IPC
} ThreadParams;

/* --- Prototypes --- */

/* Prints welcome message to user */
void welcomeMessage(void);

/*Brief: Allows user to select input and output filenames 
  Paramaters: pointer params of type ThreadParam (a struct with multiple variables) 
  Returns: void */
void fileSelection(ThreadParams *params);

/*Brief: Initializes data and utilities used in thread params
  Paramaters: pointer params of type ThreadParam (a struct with multiple variables) 
  Returns: void */
void initializeData(ThreadParams *params);

/*Brief: This thread reads data from data.txt and writes each line to a pipe
  Parameters: void pointer params 
  Returns: void */
void *ThreadA(void *params);

/*Brief: This thread reads data from pipe used in ThreadA and writes it to a shared variable
  Parameters: void pointer params
  Returns: void */
void *ThreadB(void *params);

/*Brief: This thread reads from shared variable and outputs non-header text to output.txt
  Parameters: void pointer params
  Returns: void */
void *ThreadC(void *params);

/* --- Main Code --- */
int main(int argc, char const *argv[])
{

  pthread_t tid1, tid2, tid3; //Thread IDs
  pthread_attr_t attr;        //Thread Attributes
  ThreadParams params;        //Struct to store pipes, message and semaphores. To be passed into threads.

  // Initialization
  welcomeMessage();
  fileSelection(&params);
  initializeData(&params);
  pthread_attr_init(&attr);

  // Create Threads
  if (pthread_create(&tid1, &attr, ThreadA, (void *)(&params)) != 0)
  {
    // If unsuccessful, exit
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid2, &attr, ThreadB, (void *)(&params)) != 0)
  {
    // If unsuccessful, exit
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid3, &attr, ThreadC, (void *)(&params)) != 0)
  {
    // If unsuccessful, exit
    perror("Error creating threads: ");
    exit(-1);
  }

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);

}

void welcomeMessage(void)
{
  // Print message to mains identifying purpose of program - in red bold text 
  printf("This program can be used to read data from a text file which you input and write the contents of that file into an output file which you specify.\n");
  printf("\033[1;31mNote:\033[0m If you select an output file that already exists in your directory it will be deleted.\n\n");

}

void fileSelection(ThreadParams *params)
{
  int result;
  char buffer[MAX_STR_LENGTH];
  printf("\033[1;31mIf you would like to run default setup reading from \"data.txt\" and writing to \"output.txt\", enter 'Y', otherwise, enter any key\033[0m \n\n");
  result = scanf("%s", buffer);

  if (!strcmp(buffer, "Y") || !strcmp(buffer, "y") )
  {
    printf("\n");
    strcpy(params->inputFilename, "data.txt");
    strcpy(params->outputFilename, "output.txt");
  }
  else
  {
    printf("Enter the input file name, i.e. \"data.txt\", do not include quote marks:\n");
    result = scanf("%s",params->inputFilename);
    printf("\nEnter output file name, i.e. \"output.txt\", do not include quote marks:\n");
    result = scanf("%s",params->outputFilename);
  }
  printf("Reading from: %s\n",params->inputFilename);
  printf("Writing to: %s\n", params->outputFilename);
}

void initializeData(ThreadParams *params)
{
  // Create pipe
  if (pipe(params->pipeFile) < 0)
  {
    // If unsuccessful, exit
    perror("pipe error");
    exit(1);
  }

  // Initialise C_to_A semaphore
  if(sem_init(&(params->sem_C_to_A), 0, 1))
  {
    // If semaphore is not initalised correctly, exit program
    perror("Semaphore Sem_C_to_A not initialised correctly:");
    exit(1);
  }

  // Initialise A_to_B semaphore
  if(sem_init(&(params->sem_A_to_B), 0, 0))
  {
    // If semaphore is not initialised correctly, exit program
    perror("Sem_A_to_B not initialised correctly\n");
    exit(1);
  }

  // Initialise semaphore
  if(sem_init(&(params->sem_B_to_C), 0, 0))
  {
    // If semaphore is not initialised correctly, exit program
    perror("Sem_B_to_C not initialised correctly\n");
    exit(1);
  }

}

void *ThreadA(void *params)
{
  // Declare local variables
  ThreadParams *threadA_Params = (ThreadParams*)(params); // Pointer to thread struct
	FILE* fp;                                               // File pointer to txt file to be read
	char strline[MAX_STR_LENGTH];                           // String to store text from file

  // Open data text file
	fp = fopen(threadA_Params->inputFilename, "r");

  // If file is not opened correctly, exit porgram
  if (fp == NULL)
  {
    perror("Error opening file:");
    exit(1);
  }

	while(1)
	{
    // The thread must wait for sempahore to be posted
		sem_wait(&(threadA_Params->sem_C_to_A));
		
    // Get one line from the input stream
    if (fgets(strline, MAX_STR_LENGTH, fp) == NULL)
    {
      // Has it detected end-of-file, if yes, close the file
      if (feof(fp) != 0)
      {
        printf("END OF INPUT FILE REACHED\n");
        fclose(fp);
        //successful exit
        exit(0);
      }
      // If not, there has been an error
      else
      {
        perror("fgets has encountered an error");
        exit(1);
      }
    }
    //Check is line of data has a newline character in it
    if(strstr(strline,"\n") == NULL)
      //if not append a newline character
      strcat(strline, "\n");
      
		printf("\033[1;34mIn Thread A:\033[0m writing: %s", strline);
    
    //if write is unsuccessful, exit program
		if (write(threadA_Params->pipeFile[1], strline, MAX_STR_LENGTH) < 0)
    {
      perror("Error writing to pipe\n");
      exit(1);
    }

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
    int i; // counter for for loop 

    // The thread must wait for sempahore to be posted
    sem_wait (&(threadB_Params->sem_A_to_B));

    //If reading from pipe is unsuccessful, exit program
    if (read (threadB_Params->pipeFile[0],threadB_Params->message, MAX_STR_LENGTH) < 0)
    {
      perror ("Error reading from pipe");
      exit (1);
    }
    
    printf("\033[1;34mIn Thread B:\033[0m read from pipe & put into array: ");

    //Store each character from pipe into message array
    for (i = 0; i < strlen(threadB_Params->message); i++){
      printf ("%c", threadB_Params->message[i]);}

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
  remove(threadC_Params->outputFilename);

  while(1)
  {
    // The thread must wait for sempahore to be posted
    sem_wait(&(threadC_Params->sem_B_to_C));

    // Check if message array is empty
    if (threadC_Params->message == NULL)
    {
      perror("Nothing to read from message");
      exit(1);
    }

    //Append newline character to message
    char *ch = threadC_Params->message;

    //Detects if the line of data in message is part of the header or content
    if (ContentFlag == TRUE)
    {
      //open .txt file, append to file
      fp = fopen(threadC_Params->outputFilename, "a");
      //print to txt file
      fprintf(fp, "%s", ch);
      printf("\033[1;34mIn Thread C:\033[0m written message to file\n\n");
      //close txt file
      fclose(fp);
    }
    else
      printf("\033[1;34mIn Thread C:\033[0m not content, not written to file\n\n");
    
    //Check if we are in the content region
    if (!strcmp(ch,"end_header\n"))
    {
      printf("FOUND END OF HEADER\n\n");
      //it is now content stuff
      ContentFlag = TRUE;
    }
      
    //post write sempahore
    sem_post(&(threadC_Params->sem_C_to_A));
    
  }

}