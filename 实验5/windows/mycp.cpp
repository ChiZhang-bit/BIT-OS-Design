#include<cstdio>
#include<cstdlib>
#include<Windows.h>
#include<cstring>
#include<Windowsx.h>

const int buffer_size = 1024 * 8;

void Copyfile(char* sourcefile, char* targetfile) 
{
	//打开源文件和目标文件
	HANDLE source = CreateFile(
		sourcefile,        //源文件
        GENERIC_READ,      //只读访问
        FILE_SHARE_READ,   //共享读
        NULL,              //默认安全
        OPEN_EXISTING,     //打开已存在的文件
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );
    HANDLE target = CreateFile(
        targetfile,          //目标文件路径
        GENERIC_READ | GENERIC_WRITE,  //读写访问
        FILE_SHARE_READ,   //共享读
        NULL,              //默认安全
        CREATE_ALWAYS,     //创建新文件
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
    if (source == INVALID_HANDLE_VALUE) 
    {
        //源文件错误 关闭
        printf("Source file error, source file name: %s, please check.\n", sourcefile);
        CloseHandle(source);
        CloseHandle(target);
        return;
    }

    char buffer[buffer_size];
    DWORD tp = 0;//记录每次读取的数据长度
    //ReadFile:(文件句柄，读入数据缓冲区，读入字节数，指向实际读取字节数指针)
    //当程序调用成功时,将实际读出文件的字节数保存到 &tp 指明的地址空间中。
    while (ReadFile(source, buffer, buffer_size, &tp, NULL))
    {
        //使用WriteFile()将数据写入一个文件。
        WriteFile(target, buffer, tp, &tp, NULL);
        if (tp < buffer_size)break;
    }

    //在将数据写文件之后，要改版目标文件的属性
    DWORD attr = GetFileAttributes(sourcefile);
    SetFileAttributes(targetfile, attr);
    FILETIME createtime, lastvisittime, writetime;
    GetFileTime(source, &createtime, &lastvisittime, &writetime);
    SetFileTime(target, &createtime, &lastvisittime, &writetime);

    CloseHandle(source);
    CloseHandle(target);
    return;
}

void Copydir(char* sourcedir, char* targetdir) 
{
    char fsource[buffer_size], ftarget[buffer_size];//源文件子路径，目标文件子路径
    strcpy(fsource, sourcedir);
    strcat(fsource, "\\*.*");//FindFirstTime函数所要求的
    WIN32_FIND_DATA Filecontent;//包含所有文件的属性
    HANDLE hfile = FindFirstFile(fsource, &Filecontent);//查找指定目录的第一个文件或目录并返回它的句柄
    if (hfile != INVALID_HANDLE_VALUE)
    {
        while (FindNextFile(hfile, &Filecontent))
        {
            //Filecontent为当前指向的子文件的属性结构体
            if (strcmp(Filecontent.cFileName, ".") == 0 || strcmp(Filecontent.cFileName, "..") == 0)continue;
            strcpy(fsource, sourcedir);
            strcat(fsource, "\\");
            strcat(fsource, Filecontent.cFileName);

            strcpy(ftarget, targetdir);
            strcat(ftarget, "\\");
            strcat(ftarget, Filecontent.cFileName);
            //如果是目录文件，需要创建新目录调用，递归调用CopyDir,普通文件直接CopyFile即可

            if(Filecontent.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
                CreateDirectory(ftarget, NULL);
                Copydir(fsource, ftarget);
            }
            else 
            {
                Copyfile(fsource, ftarget);
            }
        }
    }
    //打开源文件和目标文件
    HANDLE hsource = CreateFile(sourcedir,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
    HANDLE htarget = CreateFile(targetdir,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    //在将数据写文件之后，要改版目标文件的属性
    DWORD attr = GetFileAttributes(sourcedir);
    SetFileAttributes(targetdir, attr);
    FILETIME createtime, lastvisittime, writetime;
    GetFileTime(hsource, &createtime, &lastvisittime, &writetime);
    SetFileTime(htarget, &createtime, &lastvisittime, &writetime);

    CloseHandle(hsource);
    CloseHandle(htarget);
    CloseHandle(hfile);
    return;
}

int main(int argc, char* argv[])
{
    if (argc != 3) 
    {
        printf("Error input\n");
        return 0;
    }
    WIN32_FIND_DATA filecontent;
    if (FindFirstFile(argv[1], &filecontent) != INVALID_HANDLE_VALUE) 
    {
        if (filecontent.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) 
        {
            if (FindFirstFile(argv[2], &filecontent) == INVALID_HANDLE_VALUE) {
                //没有找到该目录，就要先创建一个
                CreateDirectory(argv[2], NULL);
            }
            Copydir(argv[1], argv[2]);
        }
    }
    else
    {
        printf("No Source File name: %s", argv[1]);
    }
    return 0;
}