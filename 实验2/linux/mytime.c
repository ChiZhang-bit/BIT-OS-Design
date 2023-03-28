#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>

int main(int argc, char *argv[]){
	struct timeval t_start, t_end;
	int status = -1;
	// set environment variable
	
	char path[100],dic[100];
	memset(path,0,sizeof(path));
	memset(dic,0,sizeof(dic));
	// connect string of environment path
	getcwd(dic,sizeof(dic));
	strcpy(path,getenv("PATH"));
	strcat(path,":");
	strcat(path,dic);
	//set
	setenv("PATH",path,1);

	//use fork() to create a process
	int pid = fork();
	if(pid < 0){
		// here is error
		printf("error!\n");
	}
	else if(pid == 0){
		// here is child process
		printf("this is child process.\n");
		execvp(argv[1],argv+1);
	}
	else{
		// here is parent process
		gettimeofday(&t_start,NULL);
		wait(&status);
		gettimeofday(&t_end,NULL);
		int sec = t_end.tv_sec - t_start.tv_sec;
		int usec = t_end.tv_usec - t_start.tv_usec;
		if(usec < 0){
			sec -= 1;
			usec += 1000000;
		}
		printf("The child run for %d seconds %d useconds\n",sec,usec);
	}
	return 0;
}
