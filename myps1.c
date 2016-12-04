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
#include <pwd.h>
#include <time.h>

#define INTTTY 6291456
#define PRFNSZ 16
#define PRARGSZ 80

// 프로세스 정보를 저장하는 psinfo 구조체
typedef struct psinfo {
	int     pr_flag;        
	int     pr_nlwp;
	pid_t   pr_pid;         // 프로세스의 pid
	pid_t   pr_ppid;        // 부모프로세스 pid
	pid_t   pr_pgid;        // 프로세스 그룹 리더의 pid
	pid_t   pr_sid;         // 세션 id
	uid_t   pr_uid;         // 실제 사용자 id
	uid_t   pr_euid;        // 유효 사용자 id
	gid_t   pr_gid;         
	gid_t   pr_egid;        
	uintptr_t pr_addr;      
	size_t  pr_size;        
	size_t  pr_rssize;      
	size_t  pr_pad1;
	dev_t   pr_ttydev;      // 장치 구분 번호 (터미널)
	ushort_t pr_pctcpu;     
	ushort_t pr_pctmem;     
	timestruc_t pr_start;   // 프로세스 시작 시킨 시간
	timestruc_t pr_time;    // 프로세스 실행 시간
	timestruc_t pr_ctime;  
	char    pr_fname[PRFNSZ];        // 실행된 프로세스 이름
	char    pr_psargs[PRARGSZ];     // 경로명이 포함된 프로세스 이름
	int     pr_wstat;       
	int     pr_argc;        
	uintptr_t pr_argv;      
	uintptr_t pr_envp;      
	char    pr_dmodel;      
	char    pr_pad2[3];
	int     pr_filler[7];  
} psinfo_t;

// PROC_EVENT 프로세스 찾을때마다 호출되는 형식
// pid 프로세스 아이디
// return 그만 찾으려면 1, 계속 찾으려면 0
//
typedef int (*PROC_EVENT)(struct psinfo pinfo, char option);

// function definition
int isNum(const char* value);
void help(void);
int enumProcess(PROC_EVENT callback, char option);
int printProcess(struct psinfo pinfo, char option);
void ttyCal(char* ttystr, int tty);

extern char *optarg;	// option 인자
int mysid, myuid;
struct tm *today;

int main(int argc, char **argv)
{
	int n;
	extern int optind;
	char option;
	time_t mytime;
	
	// 나의 sid, uid 얻기
	mysid = getsid(getpid());
	myuid = getuid();

	// on option
	if(argc == 1)	
	{
		printf("PID\tTTY\t TIME\tCMD\n");
		enumProcess(printProcess, 0);	
	}

	// system time, local time 가져오기
	mytime = time(NULL);
	today = localtime(&mytime);

	// 옵션과 인자에 따라 분류
	while((n = getopt(argc, argv, "jefAg:p:s:u:hzm:t:Uk")) != 1)
	{
		switch(n)
		{
			case 'j':   // pgid와 sid 포함하여 출력
				printf("PID\tPGID\tSID\tTTY\t Time\tCMD\n");
				option = 'j';
				break;
			case 'e':   // 모든 프로세스 출력
				printf("PID\tTTY\t  TIME\tCMD\n");
				option = 'e';
				break;
			case 'f':   // 프로세스의 자세한 정보 출력
				printf("UID\t\tPID\tPPID\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'f';
				break;
			case 's':	// sid가 인자와 같은 프로세스만 출력	
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 's';
				break;
			case 'A':   // 모든 프로세스 출력
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'A';
				break;
			case 'g':	// pgid가 인자와 같은 프로세스만 출력	
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'g';
				break;
			case 'u':   // uid가 인자와 같은 프로세스만 출력
				printf("PID\tTTY\t TIME\tCOMD\n");
				option = 'u';
				break;
			case 'h':   // 도움말!
				help();
				exit(1);
				break;
			case 'p':   // pid가 인자와 같은 프로세스만 출력
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'p';	
				break;
			case 'z':   // 모든 프로세스의 자세한 정보까지 출력
				printf("UID\t\tPID\tPPID\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'z';	
				break;
			case 'm':   // cmd 와 같은 프로세스 출력
				printf("UID\t\tPID\tPPID\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'm';	
				break;
			case 't':   // 프로세스의 터미널명에 인자가 포함되어 있는 프로세스만 출력 
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 't';	
				break;
			case 'U':   // 내 uid와 같은 프로세스 출력
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'U';	
				break;
			case 'k':   // 모든 프로세스를  size와 res 포함 출력
				printf("PID\tTTY\t TIME\tSIZE\tRES\tCMD\n");
				option = 'k';
				break;
		}

		enumProcess(printProcess, option);
		return 0;
	}
	return 0;	
}

// isNum 숫자로 구성된 문자열인지 검사
// value 문자열
// return 숫자로 구성되었으면 1, 아니면 0
//
int isNum(const char* value)
{
	while(*value)
	{
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
	
	// proc 디렉토리 오픈
	if((dp = opendir("/proc/")) == NULL)
	{
		perror("opendir: /proc/");
		exit(1);
	}
	
	// proc 디렉토리 안에 pid안에 psinfo라는 파일 열어 정보 얻기
	while((dent = readdir(dp)))
	{
		if(isNum(dent->d_name) == 0)
			continue;

		strcpy(path, "/proc/");
		strcat(path, dent->d_name);
		strcat(path, "/psinfo");
		if((fd = open(path, O_RDONLY)) == NULL)
		{
			continue;
		}
		
		// 이 pinfo 구조체 안에 정보 들어있음 옵션은 이 구조체 안에서 구현, 구조체를 읽음
		read(fd, (void *)&pinfo, sizeof(pinfo));

		close(fd);
		
		if(callback != NULL && (callback(pinfo, option) == 1))
		{
			break;
		}
	}
	closedir(dp);
	return 1;
}

// case마다 프로세스 정보 출력
int printProcess(struct psinfo pinfo, char option)
{
	struct passwd *pw;
	struct passwd *logpw;
	struct tm *lt;
	char tty[256] = {0,};
	char result[256] = {0,};

	// 찾은 프로세스의 uid로 passwd 파일 읽기
	pw = getpwuid(pinfo.pr_uid);

	// tty 계산
	ttyCal(tty, pinfo.pr_ttydev);

	switch(option)
	{
		case 0:		// no option
			if(pinfo.pr_sid == mysid)
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);	
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'j':
			if(pinfo.pr_sid == mysid)
			{
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
			printf("%s\t", tty);
			printf("%2d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%s\n", pinfo.pr_fname);
			break;
		case 'f':
			lt = localtime(&pinfo.pr_start.tv_sec);		
			if(pinfo.pr_sid == mysid)
			{
				printf("%s     \t", pw->pw_name);
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", pinfo.pr_ppid);
				// 만약 시간이 오늘이면 시간 출력
				if (lt->tm_year == today->tm_year && 
						lt->tm_mon == today->tm_mon && 
						lt->tm_mday == today->tm_mday)
				{
					printf("%02d:", ((pinfo.pr_start.tv_sec/3600)+9)%24);
					printf("%02d:", (pinfo.pr_start.tv_sec%3600)/60);
					printf("%02d\t", pinfo.pr_start.tv_sec%60);
				}
				// 오늘이 아니라면 월과 일 출력
				else
				{
					printf("%d월 %d일\t", lt->tm_mon+1, lt->tm_mday);
				}
					printf("%s\t", tty);
					printf("%02d:", pinfo.pr_time.tv_sec/60);
					printf("%02d\t", pinfo.pr_time.tv_sec%60);
					printf("%s\n", pinfo.pr_psargs);
			}
			break;
		case 's':
			if(atoi(optarg) == pinfo.pr_sid)
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
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
		case 'g':
			if(atoi(optarg) == pinfo.pr_pgid)
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'u':
			logpw = getpwnam(optarg);
			if(pinfo.pr_uid == logpw->pw_uid)
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'p':
			sprintf(result, "%d" ,atoi(optarg) - pinfo.pr_pid);
			if(result[0] == '0')
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'z':
			lt = localtime(&pinfo.pr_start.tv_sec);		
			printf("%s     \t", pw->pw_name);
			printf("%d\t", pinfo.pr_pid);
			printf("%d\t", pinfo.pr_ppid);
			if (lt->tm_year == today->tm_year && 
					lt->tm_mon == today->tm_mon && 
					lt->tm_mday == today->tm_mday)
			{
				printf("%02d:", ((pinfo.pr_start.tv_sec/3600)+9)%24); 
				printf("%02d:", (pinfo.pr_start.tv_sec%3600)/60);
				printf("%02d\t", pinfo.pr_start.tv_sec%60);
			}
			else
			{
				printf("%d월 %d일\t", lt->tm_mon+1, lt->tm_mday);
			}
				printf("%s\t", tty); // tty space
				printf("%02d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_psargs);
			break;
		case 'm':
			if (strstr(pinfo.pr_fname, optarg) != NULL)
			{
				lt = localtime(&pinfo.pr_start.tv_sec);		
				printf("%s     \t", pw->pw_name);
				printf("%d\t", pinfo.pr_pid);
				printf("%d\t", pinfo.pr_ppid);
				if (lt->tm_year == today->tm_year && 
						lt->tm_mon == today->tm_mon && 
						lt->tm_mday == today->tm_mday)
				{
					printf("%02d:", ((pinfo.pr_start.tv_sec/3600)+9)%24); 
					printf("%02d:", (pinfo.pr_start.tv_sec%3600)/60);
					printf("%02d\t", pinfo.pr_start.tv_sec%60);
				}
				else
				{
					printf("%d월 %d일\t", lt->tm_mon+1, lt->tm_mday);
				}
				printf("%s\t", tty);
				printf("%02d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 't':
			if (strstr(tty, optarg) != NULL)
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_fname);
			}
			break;
		case 'U':
			if(pinfo.pr_uid == myuid)
			{
				printf("%d\t", pinfo.pr_pid);
				printf("%s\t", tty);
				printf("%2d:", pinfo.pr_time.tv_sec/60);
				printf("%02d\t", pinfo.pr_time.tv_sec%60);
				printf("%s\n", pinfo.pr_psargs);
			}
			break;
		case 'k':
			printf("%d\t", pinfo.pr_pid);
			printf("%s\t", tty);
			printf("%2d:", pinfo.pr_time.tv_sec/60);
			printf("%02d\t", pinfo.pr_time.tv_sec%60);
			printf("%dK\t", pinfo.pr_size);
			printf("%dK\t", pinfo.pr_rssize);
			printf("%s\n", pinfo.pr_fname);
			break;
	}
}

void ttyCal(char* ttystr, int tty)
{
	if(tty == -1)
	{
		strcpy(ttystr, "?");
	}
	else if(tty == 0)
	{
		strcpy(ttystr, "console");
	}
	else if(tty < INTTTY)
	{
		strcpy(ttystr, "??");
	}
	else if(tty > INTTTY)
	{
		sprintf(ttystr, "pts/%d", tty-INTTTY);
	}
}

// 도움말
void help(void)
{
	printf("< ----------  사용할 수 있는 옵션들 ---------- >\n\n");
	printf("No option : \n");
	printf("\t\t현재 터미널에서 실행되는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("j option : \n");
	printf("\t\t현재 터미널에서 실행되는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  pgid  sid tty  time(min,sec) CMD\n\n");

	printf("e option : \n");
	printf("\t\t전체 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("f option : \n");
	printf("\t\t현재 터미널에서 실행되는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tloginName  pid  ppid  C  startTime  tty  time(min,sec) CMD\n\n");

	printf("A option : \n");
	printf("\t\t전체 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("s option (-s [target_sid]) : \n");
	printf("\t\tsid가 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("p option (-p [target_pid]) : \n");
	printf("\t\tpid가 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("g option (-g [target_pgid]) : \n");
	printf("\t\tpgid가 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("u option (-u [target_uid]) : \n");
	printf("\t\tloginName이 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("z option : \n");
	printf("\t\tloginName이 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");
	
	printf("m option (-m [target_CMD] : \n");
	printf("\t\tCMD가 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tuid  pid  ppid  stime(hour,min,sec)  tty  time(min,sec) CMD\n\n");

	printf("t option (-t [target_terminal] : \n");
	printf("\t\tterminal이 일치하는 프로세스에 대한 정보를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("U option : \n");
    printf("\t\t내 uid로 설정되어 있는 프로세스에 대한 정보를 출력한다.\n");
    printf("\t\t담당중인 터미널이 없거나 다르더라도 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("k option : \n");
    printf("\t\t전체 프로세스의 SIZE와 RES를 출력한다.\n");
	printf("\t\tpid  tty  time(min,sec) SIZE RES CMD\n\n");

	printf("h option : \n");
    printf("\t\t옵션 사용법을 출력한다.\n");
}
