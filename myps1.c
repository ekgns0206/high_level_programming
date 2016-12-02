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

// PROC_EVENT 프로세스 찾을때마다 호출되는 형식
// pid 프로세스 아이디
// return 그만 찾으려면 1, 계속 찾으려면 0
//
typedef int (*PROC_EVENT)(struct psinfo pinfo, char option);

extern char *optarg;	// option 인자

uid_t uid;	//myuid

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
		// 이 pinfo 구조체 안에 정보 들어있음 옵션은 이 구조체 안에서 구현
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
	int tty;
	struct passwd *pw;
	
	pw = getpwuid(pinfo.pr_uid);
	
	tty = pinfo.pr_ttydev;
	
	switch(option){
		case 0:		// no option
			if(uid == pinfo.pr_uid){	// no tty different
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'j':
			if(uid == pinfo.pr_uid){	// no tty different
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", pinfo.pr_pgid);
				printf("%d\t", pinfo.pr_sid);
				printf("%d\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'e':
			printf("%d\t", pinfo.pr_pid);
			printf("tty\t");	//tty
			printf("%2d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%s\n", pinfo.pr_fname);
			break;
		case 'f':
			printf("%s\t", pw->pw_name);
			printf("%d\t", pinfo.pr_pid);
			printf("%d\t", pinfo.pr_ppid);
			printf("%d\t", pinfo.pr_argc);
			printf("%02d:", ((pinfo.pr_start.tv_sec/3600)+9)%24);	// not today no code
			printf("%02d:", (pinfo.pr_start.tv_sec%3600)/60);
			printf("%02d\t", pinfo.pr_start.tv_sec%60);
			printf("tty\t"); // tty space
			printf("%02d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%s\n", pinfo.pr_fname);
			break;
		case 's':
			if(atoi(optarg) == pinfo.pr_sid){
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'A':
			printf("%d\t", pinfo.pr_pid);
			printf("tty\t");	//tty
			printf("%2d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%s\n", pinfo.pr_fname);
			break;
		case 'g':
			if(atoi(optarg) == pinfo.pr_pgid){
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'u':
			if(!strcmp(optarg, pw->pw_name)){
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", tty);	//tty
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
	}
}

int main(int argc, char **argv)
{
	int n;
	extern int optind;
	char option;

	uid = getuid();

	if(argc == 1){	// no option	
		printf("PID\tTTY\tTIME\tCMD\n");
		enumProcess(printProcess, 0);	
	}
	
	while((n = getopt(argc, argv, "jefAg:s:u:")) != 1){
		switch(n){
			case 'j':
				printf("PID\tPGID\tSID\tTTY\tTime\tCMD\n");
				option = 'j';
				break;
			case 'e':
				printf("PID\tTTY\tTIME\tCMD\n");
				option = 'e';
				break;
			case 'f':
				printf("UID\tPID\tPPID\tC\tSTIME\t\tTTY\tTIME\tCMD\n");
				option = 'f';
				break;
			case 's':	// sid가 인자와 같은 프로세스만 출력	
				printf("PID\tTTY\tTIME\tCMD\n");
				option = 's';
				break;
			case 'A':
				printf("PID\tTTY\tTIME\tCMD\n");
				option = 'A';
				break;
			case 'g':	// pgid가 인자와 같은 프로세스만 출력	
				printf("PID\tTTY\tTIME\tCMD\n");
				option = 'g';
				break;
			case 'u':
				printf("PID\tTTY\tTIME\tCOMD\n");
				option = 'u';
				break;
		}
		enumProcess(printProcess, option);
		return 0;
	}

	return 0;	
}
