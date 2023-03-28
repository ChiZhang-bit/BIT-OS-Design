#include <iostream>
#include <windows.h>
#include <cstring> 
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
	SYSTEMTIME starttime, endtime;//制定系统时间变量，为启动时间，结束时间 
	PROCESS_INFORMATION pro_info;//进程的信息
	STARTUPINFO pro_start_info;//进程的启动信息

	//该结构中的所有成员初始化为零，然后将cb成员设置为该结构的大小
	memset(&pro_start_info, 0, sizeof(pro_start_info));
	pro_start_info.cb = sizeof(pro_start_info);

	char cmd[1000];
	memset(cmd, 0, sizeof(cmd));
	//取出命令行中的参数
	for (int i = 1; i < argc; i++) {
		strcat(cmd, argv[i]);
		strcat(cmd, " ");
	}

	//根据命令行中的参数创建进程
	if(CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &pro_start_info, &pro_info)){
		GetSystemTime(&starttime);
	}

	WaitForSingleObject(pro_info.hProcess, INFINITE);
	GetSystemTime(&endtime);
	CloseHandle(pro_info.hProcess);

	//计算进程运行时间
	int hour = endtime.wHour - starttime.wHour;
	int minute = endtime.wMinute - starttime.wMinute;
	int second = endtime.wSecond - starttime.wSecond;
	int milliseconds = endtime.wMilliseconds - starttime.wMilliseconds;
	if (minute < 0) {
		hour -= 1;
		minute += 60;
	}
	if (second < 0) {
		minute -= 1;
		second += 60;
	}
	if (milliseconds < 0) {
		second -= 1;
		milliseconds += 1000;
	}
	cout << hour << "小时" << minute << "分" << second << "秒" << milliseconds << "毫秒";

	return 0;
}

