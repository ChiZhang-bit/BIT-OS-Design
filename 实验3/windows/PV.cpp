#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#include<string.h>

#define Producer_num 2
#define Producer_repeat 12
// ���������� �ظ�����

#define Consumer_num 3
#define Consumer_repeat 8
//���������� �ظ��˴�

#define Process_num 5
//���������

#define Buffer_size 6
//��СΪ6�Ļ�����


struct share_memory
{
    char str[Buffer_size][15];
    int head;
    int tail;
};

HANDLE s_empty, s_full, s_mutex;//�ź������
HANDLE Handle_process[Process_num + 5];//���̵ľ������
char pro_str[6][15] = { "Apple","Banana","Circle","Dog","Enter","Fruit" };

void print(struct share_memory* sm)
{
    printf("\n----------չʾĿǰ�Ļ���������---------\n");
    for (int i = 0; i < Buffer_size; i++)
    {
        if (sm->str[i][0] == '\0') {
            printf("�մ� ");
        }
        else {
            printf("%s ", sm->str[i]);
        }
    }
    printf("\n------------չʾĿǰ�������------------\n\n");
}

HANDLE MakeSharedFile()
{
    HANDLE hMapping = CreateFileMapping(
        INVALID_HANDLE_VALUE, //�����������ļ��޹ص��ڴ�ӳ��
        NULL, //ʹ��Ĭ�ϵİ�ȫ����
        PAGE_READWRITE, //��������
        0,
        sizeof(struct share_memory), //�ߵ�λ�ļ���С
        "SHARE_MEM" //�����ڴ�����
    );
    //���ļ�ӳ������һ���ӿ�ӳ�䵽�����̣������ʱ�ļ���ʼ��
    LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    //��ָ�����ڴ������
    ZeroMemory(pData, sizeof(struct share_memory));
    //ֹͣӳ��
    UnmapViewOfFile(pData);
    return(hMapping);
}

//�����ǹ����ź���������������������������򿪡��ر�;
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

//�����ӽ��̵ĺ���
void New_SubProcess(int ID)
{
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;

    char Cmdstr[105];
    char CurFile[105];

    //��ȡ��ǰ�����Ѽ���ģ����ļ�������·��
    GetModuleFileName(NULL, CurFile, sizeof(CurFile));
    sprintf(Cmdstr, "%s %d", CurFile, ID);

    //�������̲��洢���̵ľ��
    CreateProcess(NULL, Cmdstr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    Handle_process[ID] = pi.hProcess;

    return;
}

void Producer(int ID)
{
    //����ӳ��
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

        //������������ܵ��ַ��������뻺������
        srand((unsigned int)time(NULL));
        temp = rand() % 6;
        printf("%d�Ž���:��������%d�Ż����������ַ�����%s\n", ID, sm->tail, pro_str[temp]);
        strcpy(sm->str[sm->tail], pro_str[temp]);

        sm->tail = (sm->tail + 1) % Buffer_size;
        print(sm);

        ReleaseSemaphore(s_full, 1, NULL);//V(s_full)
        ReleaseSemaphore(s_mutex, 1, NULL);//V(s_mutex)
    }
    Close_sig();
    //ֹͣӳ�� �رվ��
    UnmapViewOfFile(pFile);
    CloseHandle(hMapping);
    return��
}

void Consumer(int ID)
{
    //����ӳ��
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

        printf("%d�Ž���:��������%d�Ż����������ַ�����%s\n", ID, sm->head, sm->str[sm->head]);
        strcpy(sm->str[sm->head], "\0");
        sm->head = (sm->head + 1) % Buffer_size;
        print(sm);

        ReleaseSemaphore(s_empty, 1, NULL);//V(s_empty)
        ReleaseSemaphore(s_mutex, 1, NULL);//V(s_mutex)
    }
    Close_sig();
    //ֹͣӳ�� �رվ��
    UnmapViewOfFile(pFile);
    CloseHandle(hMapping);
}


int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        //�ڸ������д���ӳ�䣬�����ź�����Ӧ�ľ��
        HANDLE hMapping = MakeSharedFile();
        Create_sig();
        printf("�����趨1�� 2�Ž���Ϊ�����߽��̣�3�� 4�� 5��Ϊ�����߽���\n");

        //����5���ӽ��̣��ֱ��Ӧ���������߽��̣����������߽��̣�
        for (int i = 1; i <= Process_num; i++)
        {
            New_SubProcess(i);
        }
        //�����̵ȴ�����ӽ��̽���
        WaitForMultipleObjects(Process_num, Handle_process + 1, TRUE, INFINITE);

        for (int i = 1; i <= Process_num; i++)
        {
            CloseHandle(Handle_process[i]);
        }
        Close_sig();
        printf("�����̽���\n");
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
