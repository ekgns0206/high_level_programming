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

// PROC_EVENT ���μ��� ã�������� ȣ��Ǵ� ����
// pid ���μ��� ���̵�
// return �׸� ã������ 1, ��� ã������ 0
//
typedef int (*PROC_EVENT)(int pid);

// isNum ���ڷ� ������ ���ڿ����� �˻�
// value ���ڿ�
// return ���ڷ� �����Ǿ����� 1, �ƴϸ� 0
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

// enumProcess �������� ���μ��� ����
// callback ã�������� ȣ��Ǵ� �Լ� ������
// �����ϸ� 0, �����ϸ� 1
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
		// �� pinfo ����ü �ȿ� ���� ������� �ɼ��� �� ����ü �ȿ��� ����
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
