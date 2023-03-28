#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#include<string.h>

#define Producer_num 2
#define Producer_repeat 12
// 两个生产者 重复六次

#define Consumer_num 3
#define Consumer_repeat 8
//三个消费者 重复八次

#define Process_num 5
//共五个进程

#define Buffer_size 6
//大小为6的缓冲区


struct share_memory
{
    char str[Buffer_size][15];
    int head;
    int tail;
};

HANDLE s_empty, s_full, s_mutex;//信号量句柄
HANDLE Handle_process[Process_num + 5];//进程的句柄数组
char pro_str[6][15] = { "Apple","Banana","Circle","Dog","Enter","Fruit" };

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

HANDLE MakeSharedFile()
{
    HANDLE hMapping = CreateFileMapping(
        INVALID_HANDLE_VALUE, //创建与物理文件无关的内存映射
        NULL, //使用默认的安全设置
        PAGE_READWRITE, //保护设置
        0,
        sizeof(struct share_memory), //高低位文件大小
        "SHARE_MEM" //共享内存名称
    );
    //将文件映射对象的一个视口映射到主进程，完成临时文件初始化
    LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    //将指定的内存块清零
    ZeroMemory(pData, sizeof(struct share_memory));
    //停止映射
    UnmapViewOfFile(pData);
    return(hMapping);
}

//下面是关于信号量句柄的三个函数——创建、打开、关闭;
void Create_sig()
{
    s_empty = CreateSemaphore(NULL, Buffer_size, Buffer_size, "SEM_EMPTY");
    s_full = CreateSemaphore(NULL, 0, Buffer_size, "SEM_FULL");
    s_mutex = CreateSemaphore(NULL, 1, 1, "SEM_MUTEX");
}


void Open_sig()
{
    s_empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SEM_EMPTY");
    s_full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SEM_FULL");
    s_mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SEM_MUTEX");
}

void Close_sig()
{
    CloseHandle(s_empty);
    CloseHandle(s_full);
    CloseHandle(s_mutex);
}

//创建子进程的函数
void New_SubProcess(int ID)
{
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;

    char Cmdstr[105];
    char CurFile[105];

    //获取当前进程已加载模块的文件的完整路径
    GetModuleFileName(NULL, CurFile, sizeof(CurFile));
    sprintf(Cmdstr, "%s %d", CurFile, ID);

    //创建进程并存储进程的句柄
    CreateProcess(NULL, Cmdstr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    Handle_process[ID] = pi.hProcess;

    return;
}

void Producer(int ID)
{
    //建立映射
    HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "SHARE_MEM");
    LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    struct share_memory* sm = (struct share_memory*)(pFile);
    Open_sig();
    for (int i = 0; i < Producer_repeat; i++)
    {
        srand((unsigned int)time(NULL));
        int temp = (rand() % 3 + 1) * 500;
        Sleep(temp);

        WaitForSingleObject(s_empty, INFINITE);//P(s_empty)
        WaitForSingleObject(s_mutex, INFINITE);//P(s_mutex)

        //随机从六个可能的字符串，放入缓冲区中
        srand((unsigned int)time(NULL));
        temp = rand() % 6;
        printf("%d号进程:生产者在%d号缓冲区生产字符串：%s\n", ID, sm->tail, pro_str[temp]);
        strcpy(sm->str[sm->tail], pro_str[temp]);

        sm->tail = (sm->tail + 1) % Buffer_size;
        print(sm);

        ReleaseSemaphore(s_full, 1, NULL);//V(s_full)
        ReleaseSemaphore(s_mutex, 1, NULL);//V(s_mutex)
    }
    Close_sig();
    //停止映射 关闭句柄
    UnmapViewOfFile(pFile);
    CloseHandle(hMapping);
    return；
}

void Consumer(int ID)
{
    //建立映射
    HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "SHARE_MEM");
    LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    struct share_memory* sm = (struct share_memory*)(pFile);
    Open_sig();
    for (int i = 0; i < Consumer_repeat; i++)
    {
        srand((unsigned int)time(NULL) + ID);
        int temp = (rand() % 3 + 1) * 1000;
        Sleep(temp);

        WaitForSingleObject(s_full, INFINITE);//P(s_full)
        WaitForSingleObject(s_mutex, INFINITE);//P(s_mutex)

        printf("%d号进程:消费者在%d号缓冲区消费字符串：%s\n", ID, sm->head, sm->str[sm->head]);
        strcpy(sm->str[sm->head], "\0");
        sm->head = (sm->head + 1) % Buffer_size;
        print(sm);

        ReleaseSemaphore(s_empty, 1, NULL);//V(s_empty)
        ReleaseSemaphore(s_mutex, 1, NULL);//V(s_mutex)
    }
    Close_sig();
    //停止映射 关闭句柄
    UnmapViewOfFile(pFile);
    CloseHandle(hMapping);
}


int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        //在父进程中创建映射，创建信号量对应的句柄
        HANDLE hMapping = MakeSharedFile();
        Create_sig();
        printf("我们设定1号 2号进程为生产者进程，3号 4号 5号为消费者进程\n");

        //创建5个子进程，分别对应两个生产者进程，三个消费者进程；
        for (int i = 1; i <= Process_num; i++)
        {
            New_SubProcess(i);
        }
        //父进程等待多个子进程结束
        WaitForMultipleObjects(Process_num, Handle_process + 1, TRUE, INFINITE);

        for (int i = 1; i <= Process_num; i++)
        {
            CloseHandle(Handle_process[i]);
        }
        Close_sig();
        printf("父进程结束\n");
        CloseHandle(hMapping);
    }
    else
    {
        int ID = atoi(argv[1]);
        if (ID <= Producer_num)
        {
            Producer(ID);
        }

        else
        {
            Consumer(ID);
        }

    }

    return 0;
}
