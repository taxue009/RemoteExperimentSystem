#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#include <pthread.h>

#include "read_config.h"

#define FPGA_ID 1
#define MYPORT 8888
#define SERV_PORT 8887
#define BACKLOG 5
#define True 1
#define False 0
#define CMD_LEN 8
#define CMD_Init 0
#define CMD_Fconf 1	
#define CMD_Oper 2
#define CMD_End 3
#define BUFFER_SIZE 1024

struct init_command{
	struct sockaddr_in sock_addr;
	unsigned char search_id;
};
struct init_command init_com;
/**
	ARM客户端初始化函数
*/
int init(){
	//初始化
	//发送socket请求
	int sockfd_r;
	struct sockaddr_in servaddr;
	unsigned short fpga_id = FPGA_ID << 8; //调节字节序
	short ask = -1;
	
	sockfd_r = socket(AF_INET,SOCK_STREAM,0);
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MYPORT); //服务器端口  htons()将主机字节顺序转换为网络字节顺序
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //服务器IP
	if(connect(sockfd_r,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
		printf("connect error!\n");
	if(write(sockfd_r,&fpga_id,sizeof(short)) <= 0)
		printf("write error\n");
	//writen(sockfd_r,&fpga_id,sizeof(short));
	while(read(sockfd_r,&ask,sizeof(short)) < 0){
		printf("no data recived!\n");
		write(sockfd_r,&fpga_id,sizeof(short));
	}
	close(sockfd_r);
	if(ask == 1)
		return True;
	else
		return False;
}
/**
	FPGA状态搜集转发线程函数
*/
static void *Sta_collection(void * init_com){
	int sockfd_r;
	struct sockaddr_in servaddr;
	//short fpga_id = FPGA_ID;
	struct init_command *Command;
	short ask = -1;
	Command = (struct init_command *)init_com;
	
	uint16_t status;
	
	sockfd_r = socket(AF_INET,SOCK_STREAM,0);
	memset(&servaddr,0,sizeof(servaddr));
	servaddr = Command->sock_addr;
	//servaddr.sin_family = AF_INET;
	//servaddr.sin_port = htons(MYPORT); //服务器端口  htons()将主机字节顺序转换为网络字节顺序
	//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //服务器IP
	if(connect(sockfd_r,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
		printf("connect error!\n");
	
	//测试
	char send_s[2] = {0x12,0x34};
	if(write(sockfd_r,send_s,2) <= 0)
		printf("write error\n");
	
	while(1){
		/** 搜集状态信息
		1、如何设定搜集频率
		2、用驱动读取状态信息
			存储在status中
		3、将状态信息发送到PC端
		write(sockfd_r,&status,sizeof(short));
		4、读取PC端的反馈（这一步是否可以省略？）
		read(sockfd_r,&ask,sizeof(short));
		*/
	}
	close(sockfd_r);
}

static void *closeSta_fun(pthread_t *Sta_c){
	if(!pthread_cancel(*Sta_c))
		printf("FPGA状态搜集转发线程已关闭。\n");
	return ((void *)0);
}

/**
1、读取PC客户端IP地址
2、读取FPGA对象状态搜集编号
3、创建FPGA状态搜集转发线程
指令码：2bit；客户端IP地址：32bit；端口号：16bit；FPGA对象状态搜集编号：8bit
*/
void CMD_Init_fun(unsigned char buff[],pthread_t Sta_c){
	printf("in CMD_Init_fun\n");
	
	struct sockaddr_in PC_liaddr;
	unsigned char search_id;
				
	memset(&PC_liaddr,0,sizeof(PC_liaddr));
	PC_liaddr.sin_family = AF_INET;
	//PC_liaddr.sin_port = htons(*(uint16_t *)(buff+5));//字节序问题
	//PC_liaddr.sin_addr.s_addr = htonl(*(uint32_t *)(buff+1));
	PC_liaddr.sin_port = *(uint16_t *)(buff+5);//字节序问题
	PC_liaddr.sin_addr.s_addr = *(uint32_t *)(buff+1);
	search_id = buff[7];
	init_com.sock_addr = PC_liaddr;
	init_com.search_id = search_id;
	
	printf("buff[7]:%2x",buff[7]);
	printf("Port:%04x",PC_liaddr.sin_port);
	printf("Port:%d",ntohs(PC_liaddr.sin_port));
	
	if(pthread_create(&Sta_c, NULL, Sta_collection, &init_com))//创建线程错误如何处理？
		printf("thread careate error\n");
	
}

/**
1、向服务器发送已准备接收指令
2、接收配置文件
3、调用驱动配置FPGA
4、向服务器确认
指令码：2bit；
*/
void CMD_Fconf_fun(int connfd){
	//FILE *fp = fopen("/config.rbf", "wb");
	FILE *fp = fopen("/home/socket_arm/config.rbf", "wb");
	if (fp == NULL)  
	{  
		printf("File:\t config.rbf Can Not Open To Write!\n");  
		exit(1);  
	}  
	// 从服务器端接收数据到buffer中
	char buffer[BUFFER_SIZE];
	memset(buffer,0,BUFFER_SIZE);  
	int length = 0;  
	while(length = recv(connfd, buffer, BUFFER_SIZE, 0) > 0)  
	{  
		if (length < 0)  
		{  
			printf("Recieve Data From Server Failed!\n");  
			break;  
		}
		if (length = 0)
		{
			printf("Recieved the file.\n");
			break;
		}
  
		int write_length = fwrite(buffer, sizeof(char), length, fp);  
		if (write_length < length)  
		{  
			printf("File:\t config.rbf Write Failed!\n");  
			break;  
		}  
		memset(buffer,0,BUFFER_SIZE);  
	}  
	printf("Recieve File:\t config.rbf From Server Finished!\n");
	fclose(fp);
				
	char filename[] = "/config.rbf";
	// if(read_config(filename) == 1)  
		printf("配置成功！\n");
}

void main(){
	/**
	模块一：系统启动
	系统在启动时除进行基本初始化操作的同时，要向服务器发送启动请求，确保服务器知道ARM客户端的信息与状态。
	1、初始化操作有哪些？
	2、向服务发送socket请求后，发送哪些信息？
	*/
	while(!init());
	/**
	模块二：建立指令接收服务器
	模块三：指令解析
	在系统启动后，建立指令接收服务器，准备接收来自服务器的操作指令。
	1、这里有四种指令，而且是顺序执行的。
	2、四种指令：
		CMD_Init：初始化准备指令（代表已有PC客户端选择本FPGA平台）
		CMD_Fconf:FPGA配置指令
		CMD_Oper:实验操作指令（开关、LED灯等操作）
		CMD_End:实验结束指令。
	*/
	int listenfd,connfd;
	struct sockaddr_in cliaddr,servaddr;
	socklen_t clilen;
	
	unsigned char buff[CMD_LEN];
	unsigned char command;
	
	pthread_t Sta_c;
	
	short ask = -1;
	
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1)
		printf("bind error");
	if(listen(listenfd,BACKLOG) == -1)
		printf("listen error");
	
	while(1){
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
		
		//单线程	本服务器只有一个客户端
		read(connfd,buff,CMD_LEN);
		//解析命令  buff
		command = (unsigned char)(buff[0] & 0x03);
		//分类执行指令
		/**
		模块四：分别执行四种指令
		*/
		switch(command){
			case CMD_Init:
				/**
				初始化准备指令：创建FPGA状态搜集转发线程，根据PC客户端IP地址请求连接PC端，以状态搜集编号为参考，将通过驱动搜集的信息转发到PC端。
				*/
				printf("CMD_Init\n");
				CMD_Init_fun(buff,Sta_c);
				ask = 1;
				write(connfd,&ask,sizeof(short));
				break;
			case CMD_Fconf:
				/**
					FPGA配置指令：接收配置文件，调用驱动进行FPGA配置。
				*/
				ask = 1;
				write(connfd,&ask,sizeof(short));
				CMD_Fconf_fun(connfd);
				write(connfd,&ask,sizeof(short));
				break;
			case CMD_Oper:
				/**
					实验操作指令：根据操作编号和相应动作，调用驱动实现相应操作。
					1、从指令中读取操作对象编号及动作
					2、调用驱动进行相应设置
					3、向服务器确认
					指令码：2bit；操作对象编号及动作：16bit(用位图形式)
				*/
				{
					uint16_t operation = *(uint16_t *)(buff+1);
					//调用驱动实现相应操作
					//应答服务器
					ask = 1;
					write(connfd,&ask,sizeof(short));
				}
				break;
			case CMD_End:
				/**
					实验结束指令：根据结束指令，结束FPGA状态搜集转发线程、断开socket连接结束。
					指令码：2bit；
					1、结束FPGA状态搜集转发线程
					2、进行一些内存的释放，设备关闭操作
					3、向服务器确认
				*/
				{
					pthread_t close_Sta;
					pthread_create(&close_Sta, NULL, closeSta_fun, &Sta_c);
					Sta_c = -1;
					ask = 1;
					write(connfd,&ask,sizeof(short));
				}
				break;
			default:
				printf("Error command !\n");
		}
		ask = -1;
		close(connfd);
	}
}