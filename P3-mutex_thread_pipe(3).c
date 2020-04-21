/*
   ----- 48450 -- week 4 lab handout 3 ------ 
By programing the handout 3, you can try to add the mutex and threads to control the critical section of two threads' operation: 
if the runnerOne is running, the runnerTwo should wait. 

Qeustion-1, why we need to add the sleep (1) in line 48?
Question-2, can we comment the lines 55 and 56?
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

typedef struct ThreadParams
{
  int pipeFile[2];
}ThreadParams;

int sum;/*this data is shared by the threads*/
/* The mutex lock */
pthread_mutex_t mutex;
/* the semaphores */
sem_t one, two;
pthread_t tid1, tid2; //Thread ID
pthread_attr_t attr; //Set of thread attributes

#define MESSLENGTH 80

//char pourIntoPipe[] = "This has been through the pipe!\n";

char collectFromPipe[MESSLENGTH];
void *runnerOne(void *param);/*threads call this function*/
void *runnerTwo(void *param);/*threads call this function*/
void initializeData();

int main(int argc, char*argv[])
{
	// used to store pipe pointer
	ThreadParams params;
	// int  fd[2];
	int success = 2;
	int success2 = 2;
	initializeData();

	if (pipe(params.pipeFile) < 0)
		perror("pipe error");

	/*get the default attributes*/
	pthread_attr_init(&attr);
	/*create the thread 1*/
	//printf("Made it here 0");
	success = pthread_create(&tid1, &attr, runnerOne, (void*)(&params));
	printf("Success of 1 = %d\n", success);
	sleep (1);
	//printf("Made it here 2");
	/*create the thread 2*/
	success2 = pthread_create(&tid2, &attr, runnerTwo, (void*)(&params));
	printf("Success of 2 = %d\n", success2);
	//printf("Made it here 3");
	/*wait for the thread to exit*/
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	//printf("Made it here 4");
	close(params.pipeFile[0]);
	close(params.pipeFile[1]);
	//printf("Made it here 5");
}

/*The thread will begin control in this function*/
void *runnerOne(void *param)
{
	int success = 100;
	//printf("Made it to runnerOne\n");
	FILE* fp;
	char str[256];
	fp = fopen("test.txt", "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    //put text file into string
    fread(str, sizeof(str)+1, 1,fp);
        //printf("This is string: %s", str);
        
    fclose(fp);

	int *fd = (int*)(((ThreadParams*)(param))->pipeFile);
	/* aquire the mutex lock */
	
	char *strline = strtok(str, "\n");
	while(1)
	{
		// success = pthread_mutex_lock(&mutex);
		sem_wait(&one);
		
		if(strline == NULL)
			{exit(1);}
		printf("thread one: write to pipe\n");
		printf("writing: %s \n\n", strline);

		// strcat(strline,"\n");
		//close(fd[0]);
		write(fd[1], strline, 80);
		// printf("success for lock is: %d\n", success);
		
		/* release the mutex lock */
		// pthread_mutex_unlock(&mutex);
		strline = NULL;
		strline = strtok(NULL, "\n");
		sem_post(&two);
	}
	
	
	//put it ino the pipe

	//pause thread A
	//unlock read semaphore

	// //get next string    
	// strline = strtok(NULL, "\n");
    
	// write content to pipe
 	// write(fd[1], pourIntoPipe, strlen(pourIntoPipe));
	// printf("success for lock is: %d\n", success);
	// printf("thread one: write to pipe\n");
	// /* release the mutex lock */
	// pthread_mutex_unlock(&mutex);

}

/*The thread will begin control in this function*/
void *runnerTwo(void *param)
{
	//("Made it to runnerTwo\n");
	int success = 100;
	int *fd = (int*)(((ThreadParams*)(param))->pipeFile);
	/* aquire the mutex lock */
	
	while (1)
	{
		// success = pthread_mutex_lock(&mutex);
		sem_wait(&two);
		// read content from the pipe
		//close(fd[1]);
		read(fd[0], collectFromPipe, 80);
		// printf("success of lock: %d\n", success);
		printf("thread two: read from pipe\n");
		printf("read: %s %zu\n\n", collectFromPipe, strlen(collectFromPipe));

		
		/* release the mutex lock */
		// pthread_mutex_unlock(&mutex);
		sem_post(&one);
	}
	

}

void initializeData() {
	sum = 0;

	/* Create the mutex lock */
	pthread_mutex_init(&mutex, NULL);

	/* Get the default attributes */
	pthread_attr_init(&attr);
	sem_init(&one, 0, 1);
  	sem_init(&two, 0, 0);
}
  // ThreadParams *myparams = (ThreadParams *)(params);
  // char *myFIFO = "/temporary/myfifo"; //temporary fifo
  // mkfifo(myFIFO, 0666);
  // FILE *input;  //input fil
  // char str[256] = {0};

  // //open data txt file
  // input = fopen("data.txt", "r");
  // //error handling
  // if (input == NULL)
  // {
  //   printf("Error opening file \n");
  //   return 0;
  // }

  // //put text file into string
  // fread(str, sizeof(str) + 1, 1, input);
  // //printf("This is string: %s", str);

  // fclose(input);

  // while (1)
  // {
  //   sem_wait(&(myparams->sem_write));
  //   //next line
  //   char *strline = strtok(str, "\n");

  //   //if null, semaphore wait
  //   if (strline == NULL)
  //     continue;

  //   printf("%s\n", strline);
  //   //put it ino the pipe
  //   myparams->pipeFile[1] = open(myFIFO, O_RDONLY);
  //   //if write is successful, continue otherwise error handle
  //   if (!write(myparams->pipeFile[1], strline, strlen(strline) + 1))
  //   {
  //     perror("write");
  //     exit(2);
  //   }

  //   close(myparams->pipeFile[1]);
  //   //unlock read semaphore
  //   sem_post(&(myparams->sem_read)); //unlocks sem_read (Thread B)
      
  //   }