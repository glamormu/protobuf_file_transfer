#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/md5.h>

int port;
char ipaddr[15];
int sockfd;
struct sockaddr_in sockaddr;
//声明SSL套接字
SSL_CTX *ctx;
SSL *ssl;

int linkS()
{
	printf("In linkS\n");
	//创建socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket\n");
		_exit(0);
	}
	//连接
	memset(&sockaddr,0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = inet_addr(ipaddr);
	if(connect(sockfd,(struct sockaddr *)&sockaddr,sizeof(sockaddr)) == -1)
	{
		perror("connect\n");
		_exit(0);
	}
	
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl,sockfd);
	if(SSL_connect(ssl) == -1)
	{
		printf("SSL connect error!\n");	
		_exit(0);
	}
	printf("Link over");
	return 1;
}

void upload_file(char *filename, char* token)
{
	printf("In upload file and filename is %s\n", filename);
	int fd;
	char cmd = 'U';

	int FileNameSize = strlen(filename);
	printf("File name size is %d\n", FileNameSize);
	char buf[1024];
	int count=0;
	struct stat fstat;
	
	//打开文件
	fd = open(filename,O_RDONLY);
	//发送命令
	SSL_write(ssl,&cmd,1);

	//send token
	int token_size = strlen(token);
	SSL_write(ssl, (void*)&token_size, sizeof(token_size));
	SSL_write(ssl, token, token_size);
	//发送文件名
	SSL_write(ssl,(void *)&FileNameSize,4);
	SSL_write(ssl,filename, FileNameSize);
	//发送文件长度
	if((stat(filename,&fstat)) == -1)
		return;
	SSL_write(ssl,(void *)&fstat.st_size,4);
	
	//发送文件数据
	while((count = read(fd,(void *)buf,1024)) > 0)
	{
		SSL_write(ssl,buf,count);	
	}
	//关闭文件
	close(fd);
}

void download_file()
{
	int fd;
	char cmd = 'D';
	char buf[1024];
	int FileNameSize;
	int filesize=0,count=0,totalrecv=0;
	int file_num=0;
	char file_name_temp[20];
	char file_d[20];
	char c;

	//发送命令
	SSL_write(ssl,&cmd,1);
	
	//接收文件总个数和文件名
	SSL_read(ssl,&file_num,4);
	printf("Total file:%d\n",file_num);
	while(file_num--)
	{
		SSL_read(ssl,file_name_temp,20);
		printf("%s\n",file_name_temp);		
	}
	printf("Download Files:");
	//输入文件名
	while((c = getchar()) != '\n' && c != EOF);
	fgets(file_d, 30, stdin);
	file_d[strlen(file_d)-1] = '\0';


	//发送文件名
	FileNameSize = strlen(file_d);
	SSL_write(ssl,(void *)&FileNameSize,4);
	SSL_write(ssl,file_d,FileNameSize);
	
	//打开并创建文件
	if((fd = open(file_d,O_RDWR|O_CREAT)) == -1)
	{
		perror("open:");
		_exit(0);	
	}
	
	//接收数据
	SSL_read(ssl,&filesize,4);
	while((count = SSL_read(ssl,(void *)buf,1024)) > 0)
	{
		write(fd,buf,count);
		totalrecv += count;
		if(totalrecv == filesize)
			break;	
	}
	
	//关闭文件
	close(fd);
}

void quit()
{
	char cmd = 'Q';
	//发送命令
	SSL_write(ssl,(void *)&cmd,1);
	//关闭及释放SSL连接
	SSL_shutdown(ssl);
	SSL_free(ssl);
	//退出
}
int main(int argc, char *argv[])
{
	int ch;
	char* upload_filename = NULL;
	char* download_filename = NULL;
	char* token = NULL;

	while((ch = getopt(argc, argv, "h:p:t:u:d:")) != -1) {
		switch (ch) {
			perror(optarg);
			case 'h':
				strcpy(ipaddr,optarg);
				printf(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				printf(optarg);
				break;
			case 't':
				token = optarg;
				break;
			case 'u':
				upload_filename = optarg;
				break;
			case 'd':
				download_filename = optarg;
				break;
			default:
				fprintf(stderr, "usage: %s -p port -t token"
						"[-u uploadfilename]"
						"[-d download filename]\n",
					argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	printf("port is %d and token is %s and upload_filename is %s\n",
			port, token, upload_filename);
//	if(optind >= argc) {
//		fprintf(stderr, "Expected argument after options\n");
//		exit(EXIT_FAILURE);
//	}
	//初始化SSl
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	//创建SSL套接字，参数表明支持版本和客户机
	ctx = SSL_CTX_new(SSLv23_client_method());
	if(ctx == NULL)
	{
		printf("Creat CTX error!!!");	
	}
	
	//建立连接
	if(linkS() == 0)
	{
		printf("user name or password error!\n");
		//关闭及释放SSL连接
		SSL_shutdown(ssl);
		SSL_free(ssl);
		_exit(0);
	}
	if (upload_filename != NULL) {
		upload_file(upload_filename, token);
	}
	//结尾操作
	close(sockfd);
	//释放CTX
	SSL_CTX_free(ctx);
	return 0;	
}
