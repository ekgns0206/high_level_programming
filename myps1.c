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

// ���μ��� ������ �����ϴ� psinfo ����ü
typedef struct psinfo {
	int     pr_flag;        
	int     pr_nlwp;
	pid_t   pr_pid;         // ���μ����� pid
	pid_t   pr_ppid;        // �θ����μ��� pid
	pid_t   pr_pgid;        // ���μ��� �׷� ������ pid
	pid_t   pr_sid;         // ���� id
	uid_t   pr_uid;         // ���� ����� id
	uid_t   pr_euid;        // ��ȿ ����� id
	gid_t   pr_gid;         
	gid_t   pr_egid;        
	uintptr_t pr_addr;      
	size_t  pr_size;        
	size_t  pr_rssize;      
	size_t  pr_pad1;
	dev_t   pr_ttydev;      // ��ġ ���� ��ȣ (�͹̳�)
	ushort_t pr_pctcpu;     
	ushort_t pr_pctmem;     
	timestruc_t pr_start;   // ���μ��� ���� ��Ų �ð�
	timestruc_t pr_time;    // ���μ��� ���� �ð�
	timestruc_t pr_ctime;  
	char    pr_fname[PRFNSZ];        // ����� ���μ��� �̸�
	char    pr_psargs[PRARGSZ];     // ��θ��� ���Ե� ���μ��� �̸�
	int     pr_wstat;       
	int     pr_argc;        
	uintptr_t pr_argv;      
	uintptr_t pr_envp;      
	char    pr_dmodel;      
	char    pr_pad2[3];
	int     pr_filler[7];  
} psinfo_t;

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
int mysid, myuid;
struct tm *today;

int main(int argc, char **argv)
{
	int n;
	extern int optind;
	char option;
	time_t mytime;
	
	// ���� sid, uid ���
	mysid = getsid(getpid());
	myuid = getuid();

	// on option
	if(argc == 1)	
	{
		printf("PID\tTTY\t TIME\tCMD\n");
		enumProcess(printProcess, 0);	
	}

	// system time, local time ��������
	mytime = time(NULL);
	today = localtime(&mytime);

	// �ɼǰ� ���ڿ� ���� �з�
	while((n = getopt(argc, argv, "jefAg:p:s:u:hzm:t:Uk")) != 1)
	{
		switch(n)
		{
			case 'j':   // pgid�� sid �����Ͽ� ���
				printf("PID\tPGID\tSID\tTTY\t Time\tCMD\n");
				option = 'j';
				break;
			case 'e':   // ��� ���μ��� ���
				printf("PID\tTTY\t  TIME\tCMD\n");
				option = 'e';
				break;
			case 'f':   // ���μ����� �ڼ��� ���� ���
				printf("UID\t\tPID\tPPID\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'f';
				break;
			case 's':	// sid�� ���ڿ� ���� ���μ����� ���	
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 's';
				break;
			case 'A':   // ��� ���μ��� ���
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'A';
				break;
			case 'g':	// pgid�� ���ڿ� ���� ���μ����� ���	
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'g';
				break;
			case 'u':   // uid�� ���ڿ� ���� ���μ����� ���
				printf("PID\tTTY\t TIME\tCOMD\n");
				option = 'u';
				break;
			case 'h':   // ����!
				help();
				exit(1);
				break;
			case 'p':   // pid�� ���ڿ� ���� ���μ����� ���
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'p';	
				break;
			case 'z':   // ��� ���μ����� �ڼ��� �������� ���
				printf("UID\t\tPID\tPPID\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'z';	
				break;
			case 'm':   // cmd �� ���� ���μ��� ���
				printf("UID\t\tPID\tPPID\tSTIME\t\tTTY\t TIME\tCMD\n");
				option = 'm';	
				break;
			case 't':   // ���μ����� �͹̳θ� ���ڰ� ���ԵǾ� �ִ� ���μ����� ��� 
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 't';	
				break;
			case 'U':   // �� uid�� ���� ���μ��� ���
				printf("PID\tTTY\t TIME\tCMD\n");
				option = 'U';	
				break;
			case 'k':   // ��� ���μ�����  size�� res ���� ���
				printf("PID\tTTY\t TIME\tSIZE\tRES\tCMD\n");
				option = 'k';
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
	while(*value)
	{
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
	
	// proc ���丮 ����
	if((dp = opendir("/proc/")) == NULL)
	{
		perror("opendir: /proc/");
		exit(1);
	}
	
	// proc ���丮 �ȿ� pid�ȿ� psinfo��� ���� ���� ���� ���
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
		
		// �� pinfo ����ü �ȿ� ���� ������� �ɼ��� �� ����ü �ȿ��� ����, ����ü�� ����
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

// case���� ���μ��� ���� ���
int printProcess(struct psinfo pinfo, char option)
{
	struct passwd *pw;
	struct passwd *logpw;
	struct tm *lt;
	char tty[256] = {0,};
	char result[256] = {0,};

	// ã�� ���μ����� uid�� passwd ���� �б�
	pw = getpwuid(pinfo.pr_uid);

	// tty ���
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
				// ���� �ð��� �����̸� �ð� ���
				if (lt->tm_year == today->tm_year && 
						lt->tm_mon == today->tm_mon && 
						lt->tm_mday == today->tm_mday)
				{
					printf("%02d:", ((pinfo.pr_start.tv_sec/3600)+9)%24);
					printf("%02d:", (pinfo.pr_start.tv_sec%3600)/60);
					printf("%02d\t", pinfo.pr_start.tv_sec%60);
				}
				// ������ �ƴ϶�� ���� �� ���
				else
				{
					printf("%d�� %d��\t", lt->tm_mon+1, lt->tm_mday);
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
				printf("%d�� %d��\t", lt->tm_mon+1, lt->tm_mday);
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
					printf("%d�� %d��\t", lt->tm_mon+1, lt->tm_mday);
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

// ����
void help(void)
{
	printf("< ----------  ����� �� �ִ� �ɼǵ� ---------- >\n\n");
	printf("No option : \n");
	printf("\t\t���� �͹̳ο��� ����Ǵ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("j option : \n");
	printf("\t\t���� �͹̳ο��� ����Ǵ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  pgid  sid tty  time(min,sec) CMD\n\n");

	printf("e option : \n");
	printf("\t\t��ü ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("f option : \n");
	printf("\t\t���� �͹̳ο��� ����Ǵ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tloginName  pid  ppid  C  startTime  tty  time(min,sec) CMD\n\n");

	printf("A option : \n");
	printf("\t\t��ü ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("s option (-s [target_sid]) : \n");
	printf("\t\tsid�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("p option (-p [target_pid]) : \n");
	printf("\t\tpid�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("g option (-g [target_pgid]) : \n");
	printf("\t\tpgid�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("u option (-u [target_uid]) : \n");
	printf("\t\tloginName�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("z option : \n");
	printf("\t\tloginName�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");
	
	printf("m option (-m [target_CMD] : \n");
	printf("\t\tCMD�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tuid  pid  ppid  stime(hour,min,sec)  tty  time(min,sec) CMD\n\n");

	printf("t option (-t [target_terminal] : \n");
	printf("\t\tterminal�� ��ġ�ϴ� ���μ����� ���� ������ ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("U option : \n");
    printf("\t\t�� uid�� �����Ǿ� �ִ� ���μ����� ���� ������ ����Ѵ�.\n");
    printf("\t\t������� �͹̳��� ���ų� �ٸ����� ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) CMD\n\n");

	printf("k option : \n");
    printf("\t\t��ü ���μ����� SIZE�� RES�� ����Ѵ�.\n");
	printf("\t\tpid  tty  time(min,sec) SIZE RES CMD\n\n");

	printf("h option : \n");
    printf("\t\t�ɼ� ������ ����Ѵ�.\n");
}
