#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	printf("hello world.\n");
	int sleeptime = atoi(argv[1]);
	sleep(sleeptime);
	return 0;
}
