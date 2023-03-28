#include<stdio.h>
#include<windows.h>

int main(int argc,char *argv[])
{
    int sleeptime = 0;
    sleeptime = atoi(argv[1]);
    printf("\n----------½«ÐÝÃß%sÃë----------\n",argv[1]);
    Sleep(sleeptime * 1000);
    return 0;
}
