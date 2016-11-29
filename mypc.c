#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <procfs.h>		// psinfo header

// PROC_EVENT 프로세스 찾을때마다 호출되는 형식
// pid 프로세스 아이디
// return 그만 찾으려면 1, 계속 찾으려면 0
//
typedef int (*PROC_EVENT)(int pid);

// isNum 숫자로 구성된 문자열인지 검사
// value 문자열
// return 숫자로 구성되었으면 1, 아니면 0
//
int isNum(const char* value)
{
	while(*value){
		if(*value < '0' || *value > '9')
			return 0;
		value++;
	}
	return 1;
}

// enumProcess 실행중인 프로세스 열거
// callback 찾을때마다 호출되는 함수 포인터
// 실패하면 0, 성공하면 1
//
int enumProcess(PROC_EVENT callback)
{
	char	path[256] = {0,};
	char	name[152] = {0,};
	const char *purename = NULL;
	struct dirent *dent = NULL;
	int fd;
	DIR *dp;
	struct psinfo pinfo;

	if((dp = opendir("/proc/")) == NULL)
	{
		perror("opendir: /proc/");
		exit(1);
	}
	
	while((dent = readdir(dp)))
	{
		if(isNum(dent->d_name) == 0)
			continue;

		strcpy(path, "/proc/");
		strcat(path, dent->d_name);
		strcat(path, "/psinfo");
		if((fd = open(path, O_RDONLY)) == NULL)
			continue;
		
		read(fd, (void *)&pinfo, sizeof(pinfo));
		// 이 pinfo 구조체 안에 정보 들어있음 옵션은 이 구조체 안에서 구현
		close(fd);
		
		if(callback != NULL && 
				(callback(pinfo.pr_pid) == 1))
			break;
	}
	closedir(dp);
	return 1;
}

int printProcess(int pid)
{
	printf("%d\n", pid);
}

int main(int argc, char **argv)
{
	printf("PID\n");
	enumProcess(printProcess);

	return 0;	
}
