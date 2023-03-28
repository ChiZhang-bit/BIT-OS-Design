#include<windows.h>
#include<stdio.h>
#include<iostream>
#include<cstdio>
#include<psapi.h>
#include"Psapi.h"
#include"tlhelp32.h"
#include"shlwapi.h"

#define DIV_GB (1024*1024*1024)
#define DIV_KB (1024)
using namespace std;

//显示当前系统中内存的使用情况
void Print_Memory_Info() {
    //使用GlobalMemoryStatusEx函数检索系统物理和虚拟内存当前使用情况的信息
    //该函数需要一个接受信息的MEMORYSTATUSEX结构的指针
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    printf("The following is system memory information.\n\n");
    printf("The usage of memory is %ld%%\n", statex.dwMemoryLoad);//物理内存使用的百分比
    printf("The total capacity of memory is %.2fGB\n", (float)statex.ullTotalPhys / DIV_GB);//实际物理内存的大小
    printf("The available memory is %.2fGB\n", (float)statex.ullAvailPhys / DIV_GB);//现在可用的物理内存大小
    printf("The total pages file is %.2fGB\n", (float)statex.ullTotalPageFile / DIV_GB);//页交换文件大小
    printf("The available pages file is %.2fGB\n", (float)statex.ullAvailPageFile / DIV_GB);//空闲的页交换空间
    printf("The total virtual space is %.2fGB\n", (float)statex.ullTotalVirtual / DIV_GB);//进程可使用虚拟机地址空间大小
    printf("The available virtual space is %.2fGB\n", (float)statex.ullAvailVirtual / DIV_GB);//空闲的虚拟地址空间大小
    printf("The available extended virtual space is %.2fGB\n", (float)statex.ullAvailExtendedVirtual / DIV_GB);//保留值，默认是0
    printf("\n\n");
}

void Print_System_Info() {
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(si));//用0来填充该内存区域
    GetSystemInfo(&si);
    printf("The following is system information.\n\n");
    printf("The page size and the granularity of page protection and commitment is %dKB\n", (int)si.dwPageSize / DIV_KB);//页面大小和页面保护和提交的粒度
    printf("The pointer to the lowest memory address accessible to applications and dynamic-link libraries (DLLs) is 0x%.8x\n", si.lpMinimumApplicationAddress);//指向应用程序和动态链接库（DLL）可访问的最低内存指针 %.8x控制长度
    printf("The pointer to the highest memory address accessible to applications and DLLs is 0x%x\n", si.lpMaximumApplicationAddress);//指向应用程序和动态链接库（DLL）可访问的最高内存指针
    printf("The number of logical processors in the current group is %d\n", si.dwNumberOfProcessors);//当前组中的逻辑处理器数
    printf("The granularity for the starting address at which virtual memory can be allocated is %dKB\n", si.dwAllocationGranularity / DIV_KB);//可以分配虚拟内存的地址的粒度
    printf("\n\n");
}

void Print_Performance_Info() {
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);
    GetPerformanceInfo(&pi, sizeof(pi));
    printf("The following is performance information.\n\n");
    printf("The number of pages currently committed by the system is %d\n", pi.CommitTotal);//系统当前提交的页数
    printf("The current maximum number of pages that can be committed by the system without extending the paging file(s) is %d\n", pi.CommitLimit);//系统在不扩展分页文件的情况下可以提交的当前最大页数
    printf("The maximum number of pages that were simultaneously in the committed state since the last system reboot is %d\n", pi.CommitPeak);//自上次系统重新启动以来同时处于提交状态的最大页数
    printf("The amount of actual physical memory in pages is %d\n", pi.PhysicalTotal);//实际物理内存
    printf("The amount of physical memory currently available is %d\n", pi.PhysicalAvailable);//当前可用的物理内存量
    printf("The amount of system cache memory is %d\n", pi.SystemCache);//系统缓存内存量
    printf("The sum of the memory currently in the paged and nonpaged kernel pools is %d\n", pi.KernelTotal);//分页和非分页内核池中当前内存的总和
    printf("The memory currently in the paged kernel pool is %d\n", pi.KernelPaged);//当前在分页内核池中的内存
    printf("The memory currently in the nonpaged kernel pool is %d\n", pi.KernelNonpaged);//当前在非分页内核池中的内存
    printf("The size of a page is %dKB", pi.PageSize / DIV_KB);//页面大小
    printf("The current number of open handles is %d\n", pi.HandleCount);//当前打开手柄的数量
    printf("The current number of processes is %d\n", pi.ProcessCount);//进程数
    printf("The current number of threads is %d\n", pi.ThreadCount);//线程数
    printf("\n\n");
}

void Print_Process_Info() {
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    //获取指定进程的快照，以及这些进程使用的堆、模块和线程
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    //获取有关系统快照中遇到的首个进程的信息
    BOOL bMore = ::Process32First(hProcessSnap, &pe);
    printf("The following is all process information.\n\n");
    printf("PID\t | Execute File\t\t\t\t\t | Working Set Size(KB)\n");
    while (bMore) {
        HANDLE hP = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
        PROCESS_MEMORY_COUNTERS pmc;
        ZeroMemory(&pmc, sizeof(pmc));
        if (GetProcessMemoryInfo(hP, &pmc, sizeof(pmc))) {
            int len = strlen(pe.szExeFile);
            printf("%d\t | %s", pe.th32ProcessID, pe.szExeFile);
            for (int i = 0; i <= 45 - len; i++) {
                printf(" ");//控制长度打表
            }
            printf("| %.2f\n", (float)pmc.WorkingSetSize / DIV_KB);
        }
        //获取下个进程
        bMore = ::Process32Next(hProcessSnap, &pe);
    }
    CloseHandle(hProcessSnap);
}

//以下三个功能函数来自书本第292页

inline bool TestSet(DWORD dwTarget, DWORD dwMask) {
    return ((dwTarget & dwMask) == dwMask);
}

#define SHOWMASK(dwTarget,type) if(TestSet(dwTarget,PAGE_##type)) {cout << "|" << #type;}

void ShowProtection(DWORD dwTarget) {
    SHOWMASK(dwTarget, READONLY);
    SHOWMASK(dwTarget, GUARD);
    SHOWMASK(dwTarget, NOCACHE);
    SHOWMASK(dwTarget, READWRITE);
    SHOWMASK(dwTarget, WRITECOPY);
    SHOWMASK(dwTarget, EXECUTE_READ);
    SHOWMASK(dwTarget, EXECUTE);
    SHOWMASK(dwTarget, EXECUTE_READWRITE);
    SHOWMASK(dwTarget, EXECUTE_WRITECOPY);
    SHOWMASK(dwTarget, NOACCESS);
}

void Print_Process_Virtual_Memory_Info() {
    int pid = 0;
    printf("Please search the infomation by PID:\n");
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(si));
    GetSystemInfo(&si);
    HANDLE hProcess;

    while (TRUE) {
        printf("Enter A PID:\t");
        scanf("%d", &pid);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (hProcess == NULL) {
            printf("No process ID:%d , Please check your PID:\n", pid);
        }
        else {
            break;
        }
    }
    
    MEMORY_BASIC_INFORMATION mbi;
    ZeroMemory(&mbi, sizeof(mbi));
    LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;
    printf("Address\t\t\t | Size\t\t | State\t | Protect\t | Type\t\t | Module\n\n");
    while (pBlock < si.lpMaximumApplicationAddress) {
        if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
            TCHAR szSize[MAX_PATH];
            StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);
            printf("%.8x-%.8x\t", (DWORD)pBlock, (DWORD)pEnd);
            printf("| %s\t", szSize);
            switch (mbi.State)
            {
            case MEM_COMMIT:
                printf("| Commited\t");
                break;
            case MEM_FREE:
                printf("| Free\t\t");
                break;
            case MEM_RESERVE:
                printf("| Reserved\t");
                break;
            default:
                break;
            }
            if (mbi.Protect == 0 && mbi.State != MEM_FREE) {
                mbi.Protect = PAGE_READONLY;
            }
            ShowProtection(mbi.Protect);
            printf("\t");
            switch (mbi.Type)
            {
            case MEM_IMAGE:
                printf("| Image\t");
                break;
            case MEM_MAPPED:
                printf("| Mapped\t");
                break;
            case MEM_PRIVATE:
                printf("| Private\t");
                break;
            default:
                break;
            }
            TCHAR szFilename[MAX_PATH];
            if (GetModuleFileName((HINSTANCE)pBlock, szFilename, MAX_PATH) > 0) {
                PathStripPath(szFilename);
                printf("\t| %s", szFilename);
            }
            printf("\n");
            pBlock = pEnd;
        }
    }
}

int main(int argc, char const* argv[])
{
    while (TRUE) {
        printf("------------------------------------------------------------------\n");
        printf("Welcome to Memory Information Sysyem Of 1120191600.\n");
        printf("1.Memory Information press: '1'\n");
        printf("2.Performance Information press '2'\n");
        printf("3.System Information press '3'\n");
        printf("4.Process Infomation press '4'\n");
        printf("5.Process Virtual Memory Information press '5'\n");
        printf("6.Quit System press '0'\n");
        printf("------------------------------------------------------------------\n\n");

        int opnum = 0;
        scanf("%d",&opnum);
        if (opnum == 1) {
            //展示当前的内存信息
            Print_Memory_Info();
        }
        if (opnum == 2) {
            Print_Performance_Info();
        }
        if (opnum == 3) {
            Print_System_Info();
        }
        if (opnum == 4) {
            Print_Process_Info();
        }
        if (opnum == 5) {
            Print_Process_Virtual_Memory_Info();
        }
        if (opnum == 0) {
            return 0;
        }
    }
    return 0;
}
