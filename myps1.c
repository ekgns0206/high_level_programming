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
typedef int (*PROC_EVENT)(struct psinfo pinfo, char option);

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
int enumProcess(PROC_EVENT callback, char option)
{
	char	path[256] = {0,};
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
				(callback(pinfo, option) == 1))
			break;
	}
	closedir(dp);
	return 1;
}

int printProcess(struct psinfo pinfo, char option)
{
	uid_t uid;
	uid = getuid();

	switch(option){
		case 0:		// no option
			if(pinfo.pr_uid == uid && pinfo.pr_fname == "sshd"){
				printf("%d\t%d\t\t%d:%02d\t%s\n",
				pinfo.pr_pid, pinfo.pr_ttydev, pinfo.pr_time.tv_sec/60,
				pinfo.pr_time.tv_sec%60, pinfo.pr_fname);
			}
			break;
		case 'a':
			break;
		case 'e':
			printf("%d\t%d\t\t%d:%02d\t%s\n",
			pinfo.pr_pid, pinfo.pr_ttydev, pinfo.pr_time.tv_sec/60,
			pinfo.pr_time.tv_sec%60, pinfo.pr_fname);
			break;
		case 'f':
			break;
		case 'l':
			break;
		case 'u':
			break;
		case 'v':
			break;
		case 'x':
			break;
	}
}

int main(int argc, char **argv)
{
	int n;
	extern char *optarg;
	extern int optind;
	char option;

	if(argc == 1){	// no option	
		printf("PID\tTTY\tTIME\tCMD\n");
		enumProcess(printProcess, 0);	
	}

	while((n = getopt(argc, argv, "aefluvx")) != 1){
		switch(n){
			case 'a':
				option = 'a';
				break;
			case 'e':
				printf("PID\tTTY\tTIME\tCMD\n");
				option = 'e';
				break;
			case 'f':
				option = 'f';
				break;
			case 'l':
				option = 'l';
				break;
			case 'u':
				option = 'u';
				break;
			case 'v':
				option = 'v';
				break;
			case 'x':
				option = 'x';
				break;
		}
		enumProcess(printProcess, option);
		return 0;
	}

	return 0;	
}
