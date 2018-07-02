#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h> 


typedef struct task 
{ 
    void *(*process) (int newfd, char *now_token); 
    int newfd;
    char *now_token;
    struct task *next; 
} Cthread_task; 


/*线程池结构*/ 
typedef struct 
{ 
    pthread_mutex_t queue_lock; 
    pthread_cond_t queue_ready; 

    /*链表结构，线程池中所有等待任务*/ 
    Cthread_task *queue_head; 

    /*是否销毁线程池*/ 
    int shutdown; 
    pthread_t *threadid; 
    
    /*线程池中线程数目*/ 
    int max_thread_num; 
    
    /*当前等待的任务数*/ 
    int cur_task_size; 

} Cthread_pool; 

static Cthread_pool *pool = NULL; 

void *thread_routine (void *arg); 


int sockfd;
struct sockaddr_in sockaddr;
struct sockaddr_in client_addr;
int sin_size;
char passwd_d[33];

SSL_CTX *ctx;

char now_token[2048]; //存放当前线程执行任务的token
char* token =NULL;

int verify_token(char* recved_token) {
	if(!recved_token || !token) {
		return -1;
	}
	return strcmp(recved_token, token);
}

void pool_init (int max_thread_num) 
{ 
    int i = 0;
    pool = (Cthread_pool *) malloc (sizeof (Cthread_pool)); 

    pthread_mutex_init (&(pool->queue_lock), NULL); 
    /*初始化条件变量*/
    pthread_cond_init (&(pool->queue_ready), NULL); 

    pool->queue_head = NULL; 

    pool->max_thread_num = max_thread_num; 
    pool->cur_task_size = 0; 

    pool->shutdown = 0; 

    pool->threadid = (pthread_t *) malloc (max_thread_num * sizeof (pthread_t)); 

    for (i = 0; i < max_thread_num; i++) 
    {  
        pthread_create (&(pool->threadid[i]), NULL, thread_routine, NULL); 
    } 
} 

void * thread_routine (void *arg) 
{ 
    printf ("starting thread 0x%x\n", pthread_self ()); 
    while (1) 
    { 
        pthread_mutex_lock (&(pool->queue_lock)); 

        while (pool->cur_task_size == 0 && !pool->shutdown) 
        { 
            printf ("thread 0x%x is waiting\n", pthread_self ()); 
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock)); 
        } 

        /*线程池要销毁了*/ 
        if (pool->shutdown) 
        { 
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/ 
            pthread_mutex_unlock (&(pool->queue_lock)); 
            printf ("thread 0x%x will exit\n", pthread_self ()); 
            pthread_exit (NULL); 
        } 

        printf ("thread 0x%x is starting to work\n", pthread_self ()); 

        /*待处理任务减1，并取出链表中的头元素*/ 
        pool->cur_task_size--; 
        Cthread_task *task = pool->queue_head; 
        pool->queue_head = task->next; 
        pthread_mutex_unlock (&(pool->queue_lock)); 

        /*调用回调函数，执行任务*/ 
        (*(task->process)) (task->newfd,task->now_token); 
        free (task); 
        task = NULL; 
    } 
    /*这一句应该是不可达的*/ 
    pthread_exit (NULL); 
}

/*向线程池中加入任务*/ 
int pool_add_task (void *(*process) (int newfd, char *now_token), int newfd, char *now_token) 
{ 
    /*构造一个新任务*/ 
    Cthread_task *task = (Cthread_task *) malloc (sizeof (Cthread_task)); 
    task->process = process; 
    task->newfd = newfd;
    task->now_token = now_token; 
    task->next = NULL;

    pthread_mutex_lock (&(pool->queue_lock)); 
    /*将任务加入到等待队列中*/ 
    Cthread_task *member = pool->queue_head; 
    if (member != NULL) 
    { 
        while (member->next != NULL) 
            member = member->next; 
        member->next = task; 
    } 
    else 
    { 
        pool->queue_head = task; 
    } 

    pool->cur_task_size++; 
    pthread_mutex_unlock (&(pool->queue_lock)); 
    //唤醒一个线程
    //加入
    pthread_cond_signal (&(pool->queue_ready)); 
    
    return 0; 
} 

int handle(char cmd,SSL *ssl,char *now_token)
{
	char filename[30]={0};
	int FileNameSize=0;
	int fd;
	int filesize=0;
	int count=0,totalrecv=0;
	char pwd[100];
	char buf[1024];
	struct stat fstat;
	int file_cnt=0;
	DIR *dir;
	struct dirent *ptr;

	switch(cmd)
	{
		case 'U':
		{
			//recv token
			int token_size = 0;
			SSL_read(ssl, &token_size, 4);
			memset(now_token, 0, 2048);
			SSL_read(ssl, (void *)now_token, token_size);
			if(verify_token(now_token)!=0) {
				fprintf(stderr, 
					"token not match! and now_token is %s and token is %s\n",
					now_token, token);
				return -1;
			}
			//接收文件名
			SSL_read(ssl, &FileNameSize, 4);
			fprintf(stderr, "FileNamesize is %d\n", FileNameSize);
			SSL_read(ssl, (void *)filename, FileNameSize);
			filename[FileNameSize]='\0';

			getcwd(pwd,sizeof(pwd));
			strcat(pwd,"/");

			strcat(pwd,filename);
			
			//创建文件
			if((fd = open(pwd,O_RDWR|O_CREAT)) == -1) {
				fprintf(stderr, "pwd is %s and file name is %s",
						pwd,filename);
				perror("creat:");
				return -1;	
			}
			//接收文件长度
			SSL_read(ssl, &filesize, 4);
			
			//接收文件
			while((count = SSL_read(ssl,(void *)buf,1024)) > 0)
			{
				write(fd,&buf,count);
				totalrecv += count;
				if(totalrecv == filesize)
					break;	
			}			
			//关闭文件
			close(fd);
		}
		break;
		
		case 'D':
		{
			//统计用户文件夹的个数
			getcwd(pwd,sizeof(pwd));
			strcat(pwd,"/");
			strcat(pwd,now_token);
			dir = opendir(pwd);
			while( (ptr = readdir(dir))!=NULL)
			{
				file_cnt++;
			}
			//发送文件个数
			SSL_write(ssl, &file_cnt, 4);
			//逐个发送文件名
			dir = opendir(pwd);
			while( (ptr = readdir(dir))!=NULL)
			{
				SSL_write(ssl, ptr->d_name, 20);
			}

			//接收文件名
			SSL_read(ssl, &FileNameSize, 4);
			SSL_read(ssl, filename, FileNameSize);
			filename[FileNameSize]='\0';

			strcat(pwd,"/");
			strcat(pwd,filename);
			
			//打开文件
			if((fd = open(pwd,O_RDONLY)) == -1)
			{
				perror("open:");
				_exit(0);	
			}
			//发送文件长度和文件名
			if((stat(pwd,&fstat)) == -1)
				return -1;
			SSL_write(ssl,&fstat.st_size,4);
			
			while((count = read(fd,(void *)buf,1024)) > 0)
			{
				SSL_write(ssl,&buf,count);	
			}
			close(fd);
		}
		break;	

		default:
		break;
	}
	return 0;
}


void *myprocess(int newfd, char *now_token)
{
	SSL *ssl;
	int tmp_fd = newfd;
	char cmd;
	//产生新的SSL
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl,tmp_fd);
	SSL_accept(ssl);
	//处理事件
	while(1)
	{
		if (SSL_read(ssl,&cmd,1) == 0) {
			continue;
		}
		
		if(cmd == 'Q') {
			SSL_shutdown(ssl);
			SSL_free(ssl);
			close(tmp_fd);
			break;	
		}
		else {
			if(!handle(cmd,ssl,now_token)) {
				SSL_shutdown(ssl);
				SSL_free(ssl);
				close(tmp_fd);
				break;	
			}
		}
	}
	return NULL;
}
int write_pipe(char* fifo_name, char* buf, size_t len){
	if(!fifo_name) {
		int pid = getpid();
		char fifo_name_default[10] = {0};
		sprintf(fifo_name_default, "%d.pipe", pid);
		fifo_name = fifo_name_default;
	}
	if(!buf) {
		fprintf(stderr, "buf is null\n");
		exit(EXIT_FAILURE);
	}
	if(access(fifo_name, F_OK) == -1) {
		if (mkfifo(fifo_name, 0777)) {
				fprintf(stderr, "mkfifo failure\n");
				exit(EXIT_FAILURE);
		}
	}
	int pipe_fd = open(fifo_name, O_WRONLY);//block here
	if(pipe_fd == -1) {
		fprintf(stderr, "open pipe file failed\n");
		exit(EXIT_FAILURE);
	}
	int res = write(pipe_fd, buf, len);
	close(pipe_fd);
	return res;
}

int main(int argc, char *args[])
{
	int ch;
	char* pipe_out = NULL;
	char* path = NULL;
	int port = 0;
	while((ch = getopt(argc, args, "p:d:t:P:")) != -1) {
		switch(ch) {
			case 'p':
				port = atoi(optarg);
				break;
			case 't':
				token = optarg;
				break;
			case 'd':
				path = optarg;
				break;
			case 'P':
				pipe_out = optarg;
				break;
			default:
				fprintf(stderr, "usage: \n%s -p port -t token"
						"-d dir -P pipe file path\n",
						args[0]);
				exit(EXIT_FAILURE);
		}
	}

	int newfd;
	int rc;
	//初始化线程池
	//pool_init(2);
	
	//SSL连接
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_server_method());
	//载入数字证书
	printf("cert loading\n");
	SSL_CTX_use_certificate_file(ctx,"./cacert.pem",SSL_FILETYPE_PEM);
	printf("cert loaded\n");
	//载入私钥
	SSL_CTX_use_PrivateKey_file(ctx,"./privkey.pem",SSL_FILETYPE_PEM);
	SSL_CTX_check_private_key(ctx);
	//创建socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket:");
		_exit(0);	
	}

	memset(&sockaddr,0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	

	if(bind(sockfd,(struct sockaddr *)&sockaddr,sizeof(sockaddr)) == -1) {
		perror("bind:");
		_exit(0);	
	}

	if(listen(sockfd,10) == -1) {
		perror("listen");	
	}

	//get port
	struct sockaddr_in sock_addr;
	socklen_t nlen = (int)sizeof(sock_addr);
	getsockname(sockfd, (struct sockaddr*)&sock_addr, &nlen);
	port =  ntohs(sock_addr.sin_port);
	char port_str[10] = {0};
	sprintf(port_str, "%d", port);
	if (write_pipe(pipe_out, port_str, strlen(port_str)) <= 0) {
		fprintf(stderr, "write pipe error\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "listnning on %s and token is %s\n",
			port_str, token);
	//while(1) {
		//连接
		if((newfd = accept(sockfd, (struct sockaddr *)(&client_addr),&sin_size)) == -1)
		{
			perror("accept:");	
			_exit(0);
		}
		printf("new fd is %d\n", newfd);
		//给线程池添加任务
//		pool_add_task(myprocess,newfd,now_token);
		myprocess(newfd, now_token);
	//}	
	close(sockfd);
	SSL_CTX_free(ctx);
	return 0;
}
