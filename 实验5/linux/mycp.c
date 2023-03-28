#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>

const int buffer_size = 1024;

void ChangeAttr(char* sourcefile, char* targetfile) 
{
    struct stat statbuf;
    lstat(sourcefile, &statbuf);
    //保存源文件属性信息结构
    chmod(targetfile, statbuf.st_mode);
    //修改文件所有者
    chown(targetfile, statbuf.st_uid, statbuf.st_gid);

    if (S_ISLNK(statbuf.st_mode))
    {
        //修改链接文件访问和修改时间
        struct timeval time_source[2];
        time_source[0].tv_sec = statbuf.st_mtime;
        time_source[1].tv_sec = statbuf.st_ctime;
        lutimes(targetfile, time_source);
    }
    else
    {
        //修改文件访问和修改时间
        struct utimbuf utime_source;
        utime_source.actime = statbuf.st_atime;
        utime_source.modtime = statbuf.st_mtime;
        utime(targetfile, &utime_source);
    }
    return;
}

void Copyfile(char* sourcefile, char* targetfile)
{
    printf("Copying file\n");
    //打开源文件
    int fd_sor = open(file_sor, O_RDONLY);
    //创建目标文件
    int fd_tar = creat(file_tar, O_WRONLY);
    //文件读写
    unsigned char buff[buffer_size];
    int len;
    while ((len = read(fd_sor, buff, buffer_size)) > 0)
    {
        printf("IN CIRCLE\n");
        write(fd_tar, buff, len);
    }
    //修改属性
    ChangeAttr(file_sor, file_tar);

    close(fd_sor);
    close(fd_tar);
}

void Copylink(char* sourcelink, char* targetlink)
{
    printf("in copy link\n");
    //读取软连接内容并复制
    unsigned char path[buffer_size];
    readlink(sourcelink, path, buffer_size);
    //复制内容到目标连接文件
    symlink(path, targetlink);
    //修改属性
    ChangeAttr(sourcelink, targetlink);
}

void Copydir(char* sourcedir,char*targetdir)
{
    printf("in copydir\n");
    //打开文件或目录
    DIR* pd_source = opendir(sourcedir);
    struct dirent* entry_source = NULL;
    struct stat statbuf;
    
    while((entry_source = readdir(pd_source))) {
	    printf("IN COPYDIR CIRCLE\n");
        //跳过当前目录和上级目录：
        if (strcmp(entry_source->d_name, ".") == 0 ||
            strcmp(entry_source->d_name, "..") == 0)
            continue;

        char temps[buffer_size], tempt[buffer_size];

        strcpy(temps, sourcedir);
        strcat(temps, "/");
        strcat(temps, entry_source->d_name);

        strcpy(tempt, targetdir);
        strcpy(tempt, "/");
        strcat(tempt, entry_source->d_name);
        //保存源文件的信息结构
        lstat(temps, &statbuf);
        //如果是LINK
        if (S_ISLNK(statbuf.st_mode))
        {
            Copylink(temps, tempt);
        }
        //如果是DIR
        else if (S_ISDIR(statbuf.st_mode)) 
        {
            mkdir(tempt, statbuf.st_mode);
	        printf("IS DIR\n");
            Copydir(temps, tempt);
        }
        //如果是REG
        else if (S_ISREG(statbuf.st_mode)) 
        {
            Copyfile(temps, tempt);
        }
        
    }
    ChangeAttr(sourcedir, targetdir);
    return;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Error input. Please Check.\n");
        return 0;
    }

    struct stat statbuf;
    lstat(argv[1], &statbuf);//返回关于文件相关的信息

    if (S_ISDIR(statbuf.st_mode))//判断该路径是否是目录
    {
        if (opendir(argv[1]) == NULL)
        {
            printf("No Source File name: %s", argv[1]);
            return 0;
        }
        if (opendir(argv[2]) == NULL)
        {
            mkdir(argv[2], statbuf.st_mode);
        }
        Copydir(argv[1], argv[2]);
    }
    printf("Copy Finished.\n");
    return 0;
}
