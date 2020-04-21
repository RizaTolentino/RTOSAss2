#include <stdio.h>
#include <string.h>

int main()
{
    FILE *fp;
    char str[256] = {0};
    int line = 0; //counts line
    char c;
    fp = fopen("test.txt", "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return(-1);
    }

    //put text file into string
    fread(str, sizeof(str)+1, 1,fp);
        //printf("This is string: %s", str);
        
    fclose(fp);

    //next line
    char *strline = strtok(str, "\n");
    
    //while 1?
    while(strline != NULL)
    {
        //put it ino the pipe
        printf("%d, %s\n", line, strline);
        //pause thread A
        //unlock read semaphore
    
        //get next string    
        strline = strtok(NULL, "\n");
        line++;
    }
    
    return 0;
}