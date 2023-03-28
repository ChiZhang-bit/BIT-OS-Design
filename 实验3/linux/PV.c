#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<unistd.h>

#define Producer_num 2
#define Producer_repeat 12
// 两个生产者 重复六次
#define Consumer_num 3
#define Consumer_repeat 8
//三个消费者 重复八次
#define Process_num 5
#define Buffer_size 6
//大小为6的缓冲区

#define SHMKEY 1024
#define SEMKEY 2048 

char pro_str[6][15] = { "Apple","Banana","Circle","Dog","Enter","Fruit" };

struct share_memory
{
	char str[Buffer_size][15];
	int head;
	int tail;
};

void print(struct share_memory* sm)
{
	printf("\n----------展示目前的缓冲区数据---------\n");
	for (int i = 0; i < Buffer_size; i++)
	{
		if (sm->str[i][0] == '\0') {
			printf("空串 ");
		}
		else {
			printf("%s ", sm->str[i]);
		}
	}
	printf("\n------------展示目前数据完毕------------\n\n");
}

int Create_sig()
{
	//创建信号量集对象 
	int semid = semget(
		SEMKEY,//通过ftok函数获取的IPC资源的key
		3,//信号量的数目 
		0666 | IPC_CREAT// 0660表示用户和同组用户有读写执行权限，其他用户没有任何访问权限 
	);
	//信号量集标识符 下标   初始值 
	semctl(semid, 0, SETVAL, 6); //empty
	semctl(semid, 1, SETVAL, 0); //full
	semctl(semid, 2, SETVAL, 1); //mutex
	return semid;
}

void P(int semid, int n)
{
	struct sembuf temp;
	//制定要对其进行的操作 
	temp.sem_num = n;//信号量编号 
	temp.sem_op = -1;// -1 -- P  || +1 -- V 
	temp.sem_flg = 0;
	//执行temp对应的sem操作 
	semop(semid, &temp, 1);
}

void V(int semid, int n)
{
	//同上 
	struct sembuf temp;
	temp.sem_num = n;
	temp.sem_op = 1;
	temp.sem_flg = 0;
	semop(semid, &temp, 1);
}

int Create_Share_Memory()
{
	//创建一个共享内存对象 
	int shmid = shmget(
		SHMKEY,//标识符关键字
		sizeof(struct share_memory),//共享存储段字节数 
		0666 | IPC_CREAT//读写权限 
	);
	struct share_memory* sm = (struct share_memory*)shmat(shmid, 0, 0);
	//初始化 
	sm->head = sm->tail = 0;
	for (int i = 0; i < Buffer_size; i++)
	{
		sm->str[i][0] = '\0';
	}
	return shmid;
}

void Producer(int ID, int shmid, int semid)
{
	struct share_memory* sm = (struct share_memory*)shmat(shmid, 0, 0);
	for (int i = 0; i < Producer_repeat; i++)
	{
		srand((unsigned)time(NULL));
		sleep(rand() % 3 + 1);

		P(semid, 0);//empty
		P(semid, 2);//mutex

		srand((unsigned)time(NULL));
		int temp = rand() % 6;
		printf("%d号进程:生产者在%d号缓冲区生产字符串：%s\n", ID, sm->tail, pro_str[temp]);
		strcpy(sm->str[sm->tail], pro_str[temp]);
		sm->tail = (sm->tail + 1) % Buffer_size;
		print(sm);

		V(semid, 1);//full
		V(semid, 2);//mutex
	}
	//断开内存共享连接 
	shmdt(sm);
}

void Consumer(int ID, int shmid, int semid)
{
	struct share_memory* sm = (struct share_memory*)shmat(shmid, 0, 0);
	for (int i = 0; i < Consumer_repeat; i++)
	{
		srand((unsigned)time(NULL));
		sleep(rand() % 3 + 1);

		P(semid, 1);//full
		P(semid, 2);//mutex

		printf("%d号进程:消费者在%d号缓冲区消费字符串：%s\n", ID, sm->head, sm->str[sm->head]);
		strcpy(sm->str[sm->head], "\0");
		sm->head = (sm->head + 1) % Buffer_size;
		print(sm);

		V(semid, 0);//empty
		V(semid, 2);//mutex
	}
	shmdt(sm);
}

int main(int argc, char* argv[])
{
	int semid = Create_sig();//信号量初始化 
	int shmid = Create_Share_Memory();// 共享缓冲区初始化 

	printf("我们设定1号 2号进程为生产者进程，3号 4号 5号为消费者进程\n");
	//创建5个子进程，分别对应两个生产者进程，三个消费者进程；
	for (int i = 1; i <= Process_num; i++)
	{
		int pid = fork();
		if (pid == 0)
		{
			//子进程
			if (i <= Producer_num)
			{
				Producer(i, shmid, semid);
			}
			else
			{
				Consumer(i, shmid, semid);
			}
			return 0;
		}
	}
	printf("这里是主进程\n");
	int status = -1;
	for (int i = 1; i <= Process_num; i++)
	{
		wait(&status);
	}
	semctl(semid, IPC_RMID, 0);
	shmctl(shmid, IPC_RMID, 0);
	printf("End\n");
	return 0;
}