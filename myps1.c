#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <utmpx.h>
#include <fcntl.h>
#include <procfs.h>		// psinfo header
#include <pwd.h>
#include <time.h>

#define INTTTY 6291456

// PROC_EVENT ���μ��� ã�������� ȣ��Ǵ� ����
// pid ���μ��� ���̵�
// return �׸� ã������ 1, ��� ã������ 0
//
typedef int (*PROC_EVENT)(struct psinfo pinfo, char option);

// function definition
int isNum(const char* value);
void help(void);
int enumProcess(PROC_EVENT callback, char option);
int printProcess(struct psinfo pinfo, char option);
void ttyCal(char* ttystr, int tty);

extern char *optarg;	// option ����
int mysid;
struct tm *today;

int main(int argc, char **argv)
{
	int n;
	extern int optind;
	char option;
	time_t mytime;

	if(argc == 1){	// no option	
		printf("PID\tTTY\t TIME\tCMD\n");
		enumProcess(printProcess, 0);	
	}
	mytime = time(NULL);
	today = localtime(&mytime);
	
	while((n = getopt(argc, argv, "jefAgp:s:u:h")) != 1){
		switch(n){
			case 'j':
				printf("PID\tPGID\tSID\tTTY\t Time\tCMD\n");
				option = 'j';
				break;
			case 'e':
				printf("PID\tTTY\t  TIME\tCMD\n");
				option = 'e';
				break;
			case 'f':
				printf("UID\tPID\tPPID\tC\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'f';
				break;
			case 's':	// sid�� ���ڿ� ���� ���μ����� ���	
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 's';
				break;
			case 'A':
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'A';
				break;
			case 'g':	// pgid�� ���ڿ� ���� ���μ����� ���	
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'g';
				break;
			case 'u':
				printf("PID\tTTY\t TIME\tCOMD\n");
				option = 'u';
				break;
			case 'h':
				help();
				exit(1);
				break;
			case 'p':
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'p';	
				break;
		}
		enumProcess(printProcess, option);
		return 0;
	}

	return 0;	
}

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
	struct passwd *pw;
	struct tm *lt;
	char tty[256] = {0,};
	char result[256] = {0,};
	pw = getpwuid(pinfo.pr_uid);	// name ������ ����
	
	mysid = getsid(getpid());
	
	ttyCal(tty, pinfo.pr_ttydev);
	switch(option){
		case 0:		// no option
			if(pinfo.pr_sid == mysid){
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);	
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'j':
			if(pinfo.pr_sid == mysid){
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", pinfo.pr_pgid);
				printf("%d\t", pinfo.pr_sid);
				printf("%s\t", tty);	
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'e':
			printf("%d\t", pinfo.pr_pid);
			printf("%s\t", tty);	//tty
			printf("%2d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%s\n", pinfo.pr_fname);
			break;
		case 'f':
			lt = localtime(&pinfo.pr_start.tv_sec);		
			if(pinfo.pr_sid == mysid){
				printf("%s\t", pw->pw_name);
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", pinfo.pr_ppid);
				printf("%d\t", pinfo.pr_argc);		// not c
				if (lt->tm_year == today->tm_year && 
						lt->tm_mon == today->tm_mon && 
						lt->tm_mday == today->tm_mday){
					printf("%02d:", ((pinfo.pr_start.tv_sec/3600)+9)%24);	// today sucess 
					printf("%02d:", (pinfo.pr_start.tv_sec%3600)/60);
					printf("%02d\t", pinfo.pr_start.tv_sec%60);
				}
				else{
					printf("%d�� %d��\t", lt->tm_mon+1, lt->tm_mday);
				}
					printf("%s\t", tty); // tty space
					printf("%02d:", pinfo.pr_time.tv_sec/60);
					printf("%02d\t", pinfo.pr_time.tv_sec%60);
					printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 's':
			if(atoi(optarg) == pinfo.pr_sid){
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'A':
			printf("%d\t", pinfo.pr_pid);
			printf("%s\t", tty);	
			printf("%2d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%s\n", pinfo.pr_fname);
			break;
		case 'g':							// ///////////////////
			sprintf(result, "%d", pinfo.pr_pgid);
			if(!strcmp(result, optarg)){
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'u':						/////////////////////////////
			if(!strcmp(pw->pw_name, optarg)){
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'p':
			sprintf(result, "%d" ,atoi(optarg) - pinfo.pr_pid);
			if(result[0] == '0'){
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
	}
}

void ttyCal(char* ttystr, int tty){

	if(tty == -1){
		strcpy(ttystr, "?");
	}
	else if(tty == 0){
		strcpy(ttystr, "console");
	}
	else if(tty < INTTTY){
		strcpy(ttystr, "??");
	}
	else if(tty > INTTTY){
		sprintf(ttystr, "pts/%d", tty-INTTTY);
	}
}

// ����
void help(void){
	printf("����� �� �ִ� �ɼǵ�\n");
	printf("No option\n");
	printf("\t\t���� �͹̳ο��� ����Ǵ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");

	printf("j option\n");
	printf("\t\t���� �͹̳ο��� ����Ǵ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  pgid  sid tty  time(min,sec) fname\n\n");

	printf("e option\n");
	printf("\t\t��ü ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");

	printf("f option\n");
	printf("\t\t���� �͹̳ο��� ����Ǵ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tloginName  pid  ppid  C  startTime  tty  time(min,sec) fname\n\n");

	printf("A option\n");
	printf("\t\t��ü ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");

	printf("s option (-s [target_sid])\n");
	printf("\t\tsid�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");

	printf("p option (-g [target_pid])\n");
	printf("\t\tpid�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");

	printf("g option (-g [target_pgid])\n");
	printf("\t\tpgid�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");

	printf("u option (-u [target_uid])\n");
	printf("\t\tloginName�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) fname\n\n");
}
