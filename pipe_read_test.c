#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

typedef char buffer_item;
#define BUFFER_SIZE 100

buffer_item buffer[BUFFER_SIZE];/* the buffer */

typedef struct ThreadParams
{
    int fd[2];//File descriptor for creating a pipe
    buffer_item inputs[100];

}ThreadParams;

pthread_t  tid1,tid2; //Thread ID
pthread_attr_t attr; //Set of thread attributes

/*This function continously reads fd[0] for any input data byte
If available, prints */

void *reader(void *param);// thread call read function
void *writer(void *param); // thread call writer function
void initializeData();

int main()
{
   int             result;
   
   ThreadParams param;
   printf("please input a string for this A thread ->B thread:\n");
   fgets(param.inputs, sizeof(param.inputs), stdin);

   void initialiszeData(); //run initialisation 

     result = pipe (param.fd);
   if (result < 0)
   {
       perror("pipe error");
       exit(1);
   }

   /*create the thread 1*/	
   pthread_create(&tid1,&attr,reader,(void*)(&param));


   /*create the thread 1*/
   pthread_create(&tid2,&attr,writer,(void*)(&param));

   /*wait for the thread to exit*/
   pthread_join(tid1,NULL);
   pthread_join(tid2,NULL);
   
   close(param.fd[0]);
   close(param.fd[1]);
}


void *reader(void *param)
{
printf ("In reading thread\n");
int *fd = (int*)(((ThreadParams*)(param))->fd);
   
   while(1){
      char    ch[255];
      int     result;
      int     i;  
      result = read (fd[0],ch,80);
    //   if (result != 1) {
    //     perror("read");
    //     // exit(4);
    //   }
      for (i = 0; i < 255; i++)
      {
        if(ch[i] !='\0')
            printf ("%c", ch[i]);
        else {
            printf("reading pipe has completed\n");
            exit (5);}
      }
    }
}
//This function continously writes Alphabet into fd[1]
//Waits if no more space is available

void *writer(void *param)
{
   int i=0, k=0;
   int result;
   buffer_item item[80];
   int *fd = (int*)(((ThreadParams*)(param))->fd);

   printf ("In writing thread\n");
   //printf("Closing pipe files\n");
   //close(fd[1]);

   /* copy the input string into local variables*/
   strcpy(item, (buffer_item*)(((ThreadParams*)(param))->inputs));

   while(item[i]!='\0')
   {
    //   result=
    write (fd[1], item, 80);
    //   if (result!=1){ perror ("write"); 
	// // exit (2);
    //}

	printf("%c", item[i]);
	i++;
   }

  /* add the '\0' in the end of the pipe */
//    result= write (fd[1], &item[i], 1);
//       if (result!=1){ perror ("write"); 
// 	exit (3);}
	
   printf("\nwriting pipe has finished\n"); 
}


void initialiszeData(){

 /*get the default attributes*/
  pthread_attr_init(&attr);
}




//void ThreadB (void *params);

//int main (void)
//{
//    ThreadParams params;
//    char pourIntoPipe[] = "This has been through the pipe!\n";
//    printf("Test\n");
//    //put data into pipe (Thread A)
//    if (pipe(params.pipeFile) < 0) /* create a pipe: pipeFile[0] is a pointer for reading and fd[1] for writing */
//        printf("pipe error");
//    //else //write to pipe
//    //{
//        printf("Entered else\n");
//        close(params.pipeFile[0]); //Jian's comment -  we don't want the reading end at all
//        write(params.pipeFile[1], pourIntoPipe, strlen(pourIntoPipe));
//        printf("Written to pipe - %s\n", pourIntoPipe);
//        //wait(NULL); //Putting Thread A on hold
//    //}
//    ThreadB(&params);
//}
//
//void ThreadB(void *params)
//{
//    int n;
//    ThreadParams *myparams = (ThreadParams *)(params);
//    //read pipe until new line
//    close(myparams->pipeFile[1]);
//    n = read(myparams->pipeFile[0], myparams->pipeFile, 255);
//    printf("The number of characters in this line is %d\n", n);
//    printf("This is read from the pipe - %s", myparams->message);
//    //store data from pipe into 2D array
//
//
//}