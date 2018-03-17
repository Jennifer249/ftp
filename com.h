#include <stdio.h>
#include <shadow.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#define _XOPEN_SOURCE
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

#define FTP_PORT 8000 
#define BUFFER_SIZE 8192
#define MAX_FILE_NAME 512
#define MAX_USER_LEN 3
#define BSIZE 100

//command code
char str220[] = "220 Service ready for new user.\r\n";
char str500[] = "500 Unknown command.\r\n";
char str331[] = "331 User name okay, need password.\r\n";
char str230[] = "230 User logged in, proceed.\r\n";
char str501[] = "501 Syntax error in parameters or arguments.\r\n";
char str530[] = "530 Not logged in.\r\n";
char str215[] = "215 UNIX.\r\n";
char str531[] = "531 Please login with USER and PASS.\r\n";
char str226[] = "226 Send ok, closing data connetion.\r\n";
char str150[] = "150 File status ok; about to open data connection.\r\n";
char str221[] = "221 Service closing control connection.\r\n";

char buffer[BUFFER_SIZE];
char headcmd[128];
char content[128];
char user[BSIZE];
char pass[BSIZE];
int con;
int state;
int pasv_state = 1;
int data_socket;
char main_cwd[BSIZE];
char type = 'A';

typedef struct Port
{
	int p1;
	int p2;
} Port;

/*
 *It's no need.  using users in system 
static const char *usernames[] =
{
	"anonymous","root","jennifer"
};
static const char *passwords[] =
{
	"anonymous","root","jennifer"
};
*/

//ftp command
void response1(char *cmd);
void ftp_pwd();
void ftp_quit();
void ftp_pasv();
void ftp_retr();
void ftp_stor();
void ftp_type();
void ftp_port();
void ftp_mkd();
void ftp_cwd();
void ftp_user();
void ftp_pass();
void ftp_list();

void send_info(int socket, char *str, int length);
int recv_info(int socket, char *str, int length);
void gethead(char *headcmd, char *cmd);
void getcon(char *content, char *cmd);
void get_port(Port *port);
int compare(char *content);
































