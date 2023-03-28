#include <iostream>
#include <windows.h>
#include <cstring> 
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
	SYSTEMTIME starttime, endtime;//�ƶ�ϵͳʱ�������Ϊ����ʱ�䣬����ʱ�� 
	PROCESS_INFORMATION pro_info;//���̵���Ϣ
	STARTUPINFO pro_start_info;//���̵�������Ϣ

	//�ýṹ�е����г�Ա��ʼ��Ϊ�㣬Ȼ��cb��Ա����Ϊ�ýṹ�Ĵ�С
	memset(&pro_start_info, 0, sizeof(pro_start_info));
	pro_start_info.cb = sizeof(pro_start_info);

	char cmd[1000];
	memset(cmd, 0, sizeof(cmd));
	//ȡ���������еĲ���
	for (int i = 1; i < argc; i++) {
		strcat(cmd, argv[i]);
		strcat(cmd, " ");
	}

	//�����������еĲ�����������
	if(CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &pro_start_info, &pro_info)){
		GetSystemTime(&starttime);
	}

	WaitForSingleObject(pro_info.hProcess, INFINITE);
	GetSystemTime(&endtime);
	CloseHandle(pro_info.hProcess);

	//�����������ʱ��
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
	cout << hour << "Сʱ" << minute << "��" << second << "��" << milliseconds << "����";

	return 0;
}

