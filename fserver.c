#include "com.h"

//创建 server_socket
int create_server(){
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(FTP_PORT);
	//创建服务器socket
	int server_socket = socket(PF_INET,SOCK_STREAM,0);
	if(server_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}

	//bind：连接socket和socket地址结构
	if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port: %d Failed!\n", FTP_PORT);
		exit(1);
	}

	//listen:监听客户端,设置监听个数
	if(listen(server_socket, MAX_USER_LEN))
	{
		printf("Server Listen Failed!");
		exit(1);
	}
	printf("等待客户端连接...\n");
	return server_socket;
}

//等待客户端连接
int accept_client(int server_socket)
{
	//定义客户端socket地址结构
	struct sockaddr_in client_addr;
	socklen_t length = sizeof(client_addr);
	//accept：连接客户端,创建客户端socket
	int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &length);
	if(client_socket < 0)
	{
		printf("Server Accept Failed!\n");
		return;
	}
	return client_socket;
}

//发送消息
void send_info(int socket, char *str, int length)
{
	int len;
	if((len = write(socket, str, length)) < 0)
	{
		perror("Send Wrong!\n");
	}
}

//接收消息
int recv_info(int socket, char *str, int length)
{
	int len;
	memset(str, 0 ,BUFFER_SIZE);
	if((len = read(socket, str, length)) < 0)
	{
		perror("Recv Wrong!\n");
	}
	str[len] = '\0';
	printf("%s\n", str);
	return 1;
}

//接受刚接入的客户端的操作
void handles(int client_socket)
{
	int bytes_read;
	char cmd[128];
	con = client_socket;
	getcwd(main_cwd, BSIZE);
	//发送220消息：准备就绪
	memset(buffer, 0, BUFFER_SIZE);
	send_info(client_socket, str220, strlen(str220));
	memset(buffer, 0, BUFFER_SIZE);
	while((bytes_read = recv_info(client_socket, buffer, BUFFER_SIZE)) >= 0)
	{
		strncpy(cmd, buffer, sizeof(cmd));
		response1(cmd);
		memset(buffer, 0, BUFFER_SIZE);
	}
}

//login
void ftp_user()
{
	printf("验证用户名\n");
	memset(buffer, 0, BUFFER_SIZE);
	int index;
	
	strncpy(user, content, strlen(content));
	user[strlen(content)] = '\0';

	//用户名核对
	if(strcmp(content, "anonymous") == 0)
	{
		send_info(con, str230, strlen(str230));
		printf("登陆成功\n");
		state = 2;
		return;
	}
	else if((index = compare_user(user)) < 0)
	{
		printf("不存在该用户名\n");
		send_info(con, str530, strlen(str530));
		return;
	}
	else {
		state = 1;
		send_info(con, str331, strlen(str331));
	}
}

//pass
void ftp_pass()
{
	int index;
	memset(buffer, 0, BUFFER_SIZE);
	strncpy(pass, content, strlen(content));
	pass[strlen(content)] = '\0';

	if((index = compare_pass(pass)) < 0)
	{
		send_info(con, str530, strlen(str530));
	}
	else
	{
		send_info(con, str230, strlen(str230));
		printf("登陆成功\n");
	}
}

void main()
{
	int pid, client_socket;
	int server_socket = create_server();
	while(1)
	{
		client_socket = accept_client(server_socket);
		printf("成功连接到一个客户端!\n");
		if((pid = fork()) < 0)
			perror("Error forking child process.\n");
		else if (pid == 0)
		{
			close(server_socket);
			handles(client_socket);
			close(client_socket);
			exit(0);
		}
		close(client_socket);
	}
	close(server_socket);
}


//非匿名操作
void response1(char *cmd)
{
	gethead(headcmd,cmd);//获得命令头部
	getcon(content,cmd);//获得命令内容

	if(strncmp(headcmd, "PASV", 4) == 0)
	{
		ftp_pasv();
	}
	else if(strncmp(headcmd, "PWD", 3) == 0)
	{
		ftp_pwd();
	}
	else if(strncmp(headcmd, "SYST", 4) == 0)
	{
		send_info(con, str215, strlen(str215));
	}
	else if(strncmp(headcmd, "LIST", 4) == 0)
	{
		ftp_list();
	}
	else if(strncmp(headcmd, "USER", 4) == 0)
	{
		ftp_user();
	}
	else if(strncmp(headcmd, "PASS", 4) == 0)
	{
		ftp_pass();
	}
	else if(strncmp(headcmd, "CWD", 3) == 0)
	{
		ftp_cwd();
	}

	else if(strncmp(headcmd, "MKD", 3) == 0)
	{
		ftp_mkd();
	}
	
	else if(strncmp(headcmd, "PORT", 4) == 0)
	{
		ftp_port();
	}
	else if(strncmp(headcmd, "STOR", 4) == 0)
	{
		ftp_stor();
	}
	
	else if(strncmp(headcmd, "RETR", 4) == 0)
	{
		ftp_retr();
	}
	
	else if(strncmp(headcmd, "TYPE", 4) == 0)
	{
		ftp_type();
	}
	
	else if(strncmp(headcmd, "QUIT", 4) == 0)
	{
		ftp_quit();
	}
	else
	{
		send_info(con, str500, strlen(str500));
	}
}



//获得命令头部
void gethead(char *headcmd, char *cmd)
{
	memset(headcmd, 0, 128);
	int len = strlen(cmd);
	strncpy(headcmd, cmd, len);
	int i = 0;
	for(; i < len; i++)
	{
		if(headcmd[i] == ' ')
		{
			headcmd[i] = '\0';
			break;
		}
	}
}

//获得命令内容
void getcon(char *content, char *cmd)
{
	memset(content, 0, 128);
	int len = strlen(cmd);
	int i = 0;
	int j = 0;
	for(; i < len; i++)
	{
		if(cmd[i] == ' ')
		{
			i++;
			len = len - i;
			break;
		}
	}
	for(; j < len; j++, i++)
	{
		content[j] = cmd[i];
	}
	int l = strlen(content);
	content[l-2] = '\0';
}


//查找是否存在姓名和密码
int lookup(char *needle, const char **haystack, int count)
{
	int i;
	for(i = 0; i < count; i++)
	{
		if(strstr(needle, haystack[i]) != NULL)
		{
			return i;
		}
	}
	return -1;
}

//pwd
void ftp_pwd()
{
	char cwd[128];
	char curr[128];
	memset(cwd, 0, 128);
	snprintf(cwd, 128, "257 \"%s\" is current location.\r\n",getcwd(curr,128));
	send_info(con, cwd, strlen(cwd));
}
	
//get_port
void get_port(Port *port)
{
	srand(time(NULL));
	port->p1 = 128 + (rand() % 64);
	port->p2 = rand() % 0xff;
}

//get_ip
void get_ip(int *ip)
{
	socklen_t addr_size = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	getsockname(con, (struct sockaddr *)&addr, &addr_size);
	char *host = inet_ntoa(addr.sin_addr);
	sscanf(host, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

//pasv
void ftp_pasv()
{
	if(state == 1)
	{
		pasv_state = 1;
		//获得本地ip和端口
		int ip[4];
		Port *port = malloc(sizeof(Port));
		get_port(port);
		get_ip(ip);
		char str227[] = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n";
		struct sockaddr_in data_addr;
		bzero(&data_addr, sizeof(data_addr));
		data_addr.sin_family = AF_INET;
		data_addr.sin_addr.s_addr = htons(INADDR_ANY);
		data_addr.sin_port = htons((256*port->p1) + port->p2);
		//关闭之前的
		close(data_socket);
		//创建数据端口
		data_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(data_socket < 0)
		{
			printf("Create Socket Failed!\n");
			exit(1);
		}
		//bind:
		if(bind(data_socket, (struct sockaddr *)&data_addr, sizeof(struct sockaddr)))
		{
			printf("Server Bind Port: %d Failed!\n",(256*port->p1+port->p2));
			exit(1);
		}
		//listen:
		if(listen(data_socket,MAX_USER_LEN))
		{
			printf("Server Listen Failed!");
			exit(1);
		}
		//printf("port: %d\n", 256*port->p1+port->p2);
		memset(buffer, 0, BUFFER_SIZE);
		sprintf(buffer, str227, ip[0], ip[1], ip[2], ip[3], port->p1,port->p2);
		puts(buffer);
		send_info(con, buffer, strlen(buffer));

	}
	else
	{
		//printf("%s", str531);
		send_info(con, str531, strlen(str531));
	}
}		

//list
void ftp_list()
{

	if(state == 1)
	{
		int list_client_socket;
		//主动模式监听
		if(pasv_state == 1)
		{
			list_client_socket = accept_client(data_socket);
		}
		else
		{
			list_client_socket = data_socket;
		}
		char data[MAX_FILE_NAME];
		size_t num_read;
		FILE* fd;
		int rs = system("ls -l | cat -A | tr '$' '\r\n'> tmp.txt");
		if(rs < 0)
		{
			exit(1);
		}

		fd = fopen("tmp.txt","r");
		if(!fd)
			exit(1);
		fseek(fd, SEEK_SET, 0);
		memset(data, 0, MAX_FILE_NAME);

		send_info(con, str150, strlen(str150));
		printf("%s\n", str150);
		while((num_read = fread(data, 1,MAX_FILE_NAME, fd)) > 0)
		{
		//	printf("%s..%d\n", data,strlen(data));
			send_info(list_client_socket, data, num_read);
			memset(data, 0, MAX_FILE_NAME);
		}
		send_info(con, str226, strlen(str226));
		printf("%s\n", str226);
		fclose(fd);
		close(list_client_socket);//关闭客户端的数据端口
		if(pasv_state == 1)
			close(data_socket);
	}
	else
	{
		printf("%s", str531);
		send_info(con, str531, strlen(str531));
	}
}
/*
//用到被动链接
void ftp_list()
{
   send_info(con, str150, strlen(str150));

   	int t_data_sock;
int DIR_INFO =100;
	struct sockaddr_in client;
	int sin_size=sizeof(struct sockaddr_in);
	if((t_data_sock=accept(data_socket,(struct sockaddr *)&client,&sin_size))==-1)
	{
		perror("accept error");
		return;
	}

	FILE *pipe_fp;
	char t_dir[DIR_INFO];
	char list_cmd_info[DIR_INFO];
	snprintf(list_cmd_info, DIR_INFO, "ls -l %s", getcwd(t_dir, DIR_INFO));

   	if ((pipe_fp = popen(list_cmd_info, "r")) == NULL)
    {
		printf("pipe open error in cmd_list\n");
		return ;
    }
   	//printf("pipe open successfully!, cmd is %s\n", list_cmd_info);

	char t_char;
   	while ((t_char = fgetc(pipe_fp)) != EOF)
    {
		printf("%c", t_char);
		write(t_data_sock,&t_char, 1);
    }
		send_info(con, str226, strlen(str226));
   	pclose(pipe_fp);
   	printf("close pipe successfully!\n");
   	close(t_data_sock);
	printf("%s close data successfully!\n",str226);
	close(data_socket);
}

*/

//type
void ftp_type()
{
	if(state == 1)
	{
		char *message;
		if(strncmp(content,"I",1) == 0)
		{
			message = "200 Switching to Binary mode.\r\n";
			type = 'I';
		}
		else if(strncmp(content,"A",1) == 0)
		{
			 message = "200 Switching to ASCII mode.\r\n";
			 type = 'A';
		}
		else
		{
			message = "504 Command not implemented for that parameter.\r\n";
		}
		send_info(con, message, strlen(message));
		printf("%s\n",message);
	}
	else
		send_info(con, str531, strlen(str531));
}

//quit
void ftp_quit()
{
	send_info(con, str221, strlen(str221));
	close(con);
	exit(0);	//0表示正常退出
}

//retr
void ftp_retr()
{

	if(state == 1)
	{
		int retr_client_socket;
		if(pasv_state == 1)
			retr_client_socket = accept_client(data_socket);
		else
			retr_client_socket = data_socket;
		char data[MAX_FILE_NAME];
		size_t num_read;
		FILE *fd = fopen(content, "r");
		if(fd == NULL)
		{
			char str550[] = "550 File not found!\r\n";
			send_info(con, str550, strlen(str550));
		}
		else
		{
			char str150[] = "150 File status okay.\r\n";
			send_info(con, str150, strlen(str150));
			do
			{
				num_read = fread(data, sizeof(char), MAX_FILE_NAME, fd);
				if(num_read < 0)
				{
					printf("Error in fread().\r\n");
				}
				send_info(retr_client_socket, data, num_read);
			}while(num_read > 0);
			send_info(con, str226, strlen(str226));
			fclose(fd);
			close(retr_client_socket);
			if(pasv_state == 1)
				close(data_socket);
		}
	}
	else
	{
		send_info(con, str531, strlen(str531));
	}
}

//ftp_stor
void ftp_stor()
{
	if(state == 1)	
	{
		int stor_client_socket;
		if(pasv_state == 1)
			stor_client_socket = accept_client(data_socket);
		else  
			stor_client_socket = data_socket;

		char data[MAX_FILE_NAME];
		size_t num_recv;
		size_t num_write;
		FILE *fp = fopen(content, "w");
		if(NULL == fp)
		{
			perror("ftp_stor:fopen!\n");
		}
		bzero(data, MAX_FILE_NAME);
		char str125[] = "125 Data connection already open; trandfer starting.\n";
		send_info(con, str125, strlen(str125));
		while(num_recv = recv(stor_client_socket, data, MAX_FILE_NAME, 0))
		{
			if(num_recv < 0)
			{
				perror("ftp_stor:num_write.\n");
			}
			num_write = fwrite(data, 1, strlen(data), fp);
			if(num_write < num_recv)
			{
				perror("num_write != num_recv");
			}
			bzero(data, MAX_FILE_NAME);
		}
		char str226[] = "226 File send ok.\r\n";
		send_info(con, str226, strlen(str226));
		fclose(fp);
		close(stor_client_socket);
		if(pasv_state == 1)
			close(data_socket);
	}
	else
	{
		send_info(con, str531, strlen(str531));
	}
}

//ftp_port
void ftp_port()
{
	pasv_state = 2;
	int port[2];
	char str200[] = "200 Port command successful.\r\n";
	int ip[4];
	sscanf(content,"%d,%d,%d,%d,%d,%d", &ip[0],&ip[1],&ip[2],&ip[3],&port[0],&port[1]);
	char local[128]; 
	snprintf(local, 128, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	struct sockaddr_in data_addr;
	bzero(&data_addr, sizeof(data_addr));
	data_addr.sin_family = AF_INET;
	data_addr.sin_addr.s_addr = htons(INADDR_ANY);
	data_addr.sin_port = htons(0);
	data_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(data_socket < 0)
	{
		printf("Create Data_socket Failed!\n");
		exit(1);
	}

	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	if(inet_pton(AF_INET, local, &client_addr.sin_addr) == 0)
	{
		printf("client_port IP Adress Error!\n");
	}
	client_addr.sin_port = htons(port[0]*256 + port[1]);
	socklen_t client_addr_length = sizeof(client_addr);
	if(connect(data_socket, (struct sockaddr *)&client_addr, client_addr_length) < 0)
	{
		printf("Can Not Connect!\n");
		exit(1);
	}
	printf("%s",str220);
	send_info(con, str220, strlen(str220));
}
//MKD
void ftp_mkd()
{
	if(state == 1)
	{
		char cwd[BSIZE];
		char res[BSIZE];
		memset(cwd, 0, BSIZE);
		memset(res, 0, BSIZE);
		getcwd(cwd, BSIZE);
		
		//绝对路径
		if(content[0] == '/')
		{
			if(strncmp(main_cwd, content, strlen(main_cwd)) == 0)
			{
				if(mkdir(content, S_IRWXU) == 0)
				{
					sprintf(res,"257 \"%s\" new directory created.\r\n", content);
					send_info(con, res, strlen(res));
				}
				else
				{	
					char str550[] = "550 Failed to create directory.Check path or permissions.\r\n";
					send_info(con, str550, strlen(str550));
				}
			}
			else
			{
				char str550[] = "550 Haven't authority.\r\n";
				send_info(con, str550, strlen(str550));
			}
		}
		else
		{
			if(mkdir(content,S_IRWXU) == 0)
			{
				sprintf(res, "257 \"%s/%s\" new directory created.\r\n",cwd,content);
				send_info(con, res, strlen(res));
			}
			else
			{
				char str550[] = "550 Failed to create directory.\r\n";
				send_info(con, str550, strlen(str550));
			}
		}
	}
	else
		send_info(con, str531, strlen(str531));
}

//cwd
void ftp_cwd()
{
	if(state == 1)
	{
		if(chdir(content) == 0)
		{
			char str250[] = "250 Directory successfully changed.\r\n";
			send_info(con, str250, strlen(str250));
		}
		else
		{
			char str550[] = "550 Failed to change directory.\r\n";
			send_info(con, str550, strlen(str550));
		}
	}
	else
		send_info(con, str531, strlen(str531));
}

//用户名验证
int compare_user(char *user)
{
	struct spwd *sp;
	sp = getspnam(user);
	if(sp == NULL)
	{
		printf("get spentry error\n");
		return -1;
	}
	return 1;
}

//用户密码验证
int compare_pass(char *pass)
{
	struct spwd *sp;
	sp = getspnam(user);	
	if(strcmp(sp->sp_pwdp, (char *)crypt(pass, sp->sp_pwdp)) == 0)
		return 1;
	else
		return -1;

}






















