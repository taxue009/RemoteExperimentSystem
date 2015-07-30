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
	ARM�ͻ��˳�ʼ������
*/
int init(){
	//��ʼ��
	//����socket����
	int sockfd_r;
	struct sockaddr_in servaddr;
	unsigned short fpga_id = FPGA_ID << 8; //�����ֽ���
	short ask = -1;
	
	sockfd_r = socket(AF_INET,SOCK_STREAM,0);
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MYPORT); //�������˿�  htons()�������ֽ�˳��ת��Ϊ�����ֽ�˳��
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //������IP
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
	FPGA״̬�Ѽ�ת���̺߳���
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
	//servaddr.sin_port = htons(MYPORT); //�������˿�  htons()�������ֽ�˳��ת��Ϊ�����ֽ�˳��
	//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //������IP
	if(connect(sockfd_r,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
		printf("connect error!\n");
	
	//����
	char send_s[2] = {0x12,0x34};
	if(write(sockfd_r,send_s,2) <= 0)
		printf("write error\n");
	
	while(1){
		/** �Ѽ�״̬��Ϣ
		1������趨�Ѽ�Ƶ��
		2����������ȡ״̬��Ϣ
			�洢��status��
		3����״̬��Ϣ���͵�PC��
		write(sockfd_r,&status,sizeof(short));
		4����ȡPC�˵ķ�������һ���Ƿ����ʡ�ԣ���
		read(sockfd_r,&ask,sizeof(short));
		*/
	}
	close(sockfd_r);
}

static void *closeSta_fun(pthread_t *Sta_c){
	if(!pthread_cancel(*Sta_c))
		printf("FPGA״̬�Ѽ�ת���߳��ѹرա�\n");
	return ((void *)0);
}

/**
1����ȡPC�ͻ���IP��ַ
2����ȡFPGA����״̬�Ѽ����
3������FPGA״̬�Ѽ�ת���߳�
ָ���룺2bit���ͻ���IP��ַ��32bit���˿ںţ�16bit��FPGA����״̬�Ѽ���ţ�8bit
*/
void CMD_Init_fun(unsigned char buff[],pthread_t Sta_c){
	printf("in CMD_Init_fun\n");
	
	struct sockaddr_in PC_liaddr;
	unsigned char search_id;
				
	memset(&PC_liaddr,0,sizeof(PC_liaddr));
	PC_liaddr.sin_family = AF_INET;
	//PC_liaddr.sin_port = htons(*(uint16_t *)(buff+5));//�ֽ�������
	//PC_liaddr.sin_addr.s_addr = htonl(*(uint32_t *)(buff+1));
	PC_liaddr.sin_port = *(uint16_t *)(buff+5);//�ֽ�������
	PC_liaddr.sin_addr.s_addr = *(uint32_t *)(buff+1);
	search_id = buff[7];
	init_com.sock_addr = PC_liaddr;
	init_com.search_id = search_id;
	
	printf("buff[7]:%2x",buff[7]);
	printf("Port:%04x",PC_liaddr.sin_port);
	printf("Port:%d",ntohs(PC_liaddr.sin_port));
	
	if(pthread_create(&Sta_c, NULL, Sta_collection, &init_com))//�����̴߳�����δ���
		printf("thread careate error\n");
	
}

/**
1���������������׼������ָ��
2�����������ļ�
3��������������FPGA
4���������ȷ��
ָ���룺2bit��
*/
void CMD_Fconf_fun(int connfd){
	//FILE *fp = fopen("/config.rbf", "wb");
	FILE *fp = fopen("/home/socket_arm/config.rbf", "wb");
	if (fp == NULL)  
	{  
		printf("File:\t config.rbf Can Not Open To Write!\n");  
		exit(1);  
	}  
	// �ӷ������˽������ݵ�buffer��
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
		printf("���óɹ���\n");
}

void main(){
	/**
	ģ��һ��ϵͳ����
	ϵͳ������ʱ�����л�����ʼ��������ͬʱ��Ҫ�������������������ȷ��������֪��ARM�ͻ��˵���Ϣ��״̬��
	1����ʼ����������Щ��
	2���������socket����󣬷�����Щ��Ϣ��
	*/
	while(!init());
	/**
	ģ���������ָ����շ�����
	ģ������ָ�����
	��ϵͳ�����󣬽���ָ����շ�������׼���������Է������Ĳ���ָ�
	1������������ָ�������˳��ִ�еġ�
	2������ָ�
		CMD_Init����ʼ��׼��ָ���������PC�ͻ���ѡ��FPGAƽ̨��
		CMD_Fconf:FPGA����ָ��
		CMD_Oper:ʵ�����ָ����ء�LED�ƵȲ�����
		CMD_End:ʵ�����ָ�
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
		
		//���߳�	��������ֻ��һ���ͻ���
		read(connfd,buff,CMD_LEN);
		//��������  buff
		command = (unsigned char)(buff[0] & 0x03);
		//����ִ��ָ��
		/**
		ģ���ģ��ֱ�ִ������ָ��
		*/
		switch(command){
			case CMD_Init:
				/**
				��ʼ��׼��ָ�����FPGA״̬�Ѽ�ת���̣߳�����PC�ͻ���IP��ַ��������PC�ˣ���״̬�Ѽ����Ϊ�ο�����ͨ�������Ѽ�����Ϣת����PC�ˡ�
				*/
				printf("CMD_Init\n");
				CMD_Init_fun(buff,Sta_c);
				ask = 1;
				write(connfd,&ask,sizeof(short));
				break;
			case CMD_Fconf:
				/**
					FPGA����ָ����������ļ���������������FPGA���á�
				*/
				ask = 1;
				write(connfd,&ask,sizeof(short));
				CMD_Fconf_fun(connfd);
				write(connfd,&ask,sizeof(short));
				break;
			case CMD_Oper:
				/**
					ʵ�����ָ����ݲ�����ź���Ӧ��������������ʵ����Ӧ������
					1����ָ���ж�ȡ���������ż�����
					2����������������Ӧ����
					3���������ȷ��
					ָ���룺2bit�����������ż�������16bit(��λͼ��ʽ)
				*/
				{
					uint16_t operation = *(uint16_t *)(buff+1);
					//��������ʵ����Ӧ����
					//Ӧ�������
					ask = 1;
					write(connfd,&ask,sizeof(short));
				}
				break;
			case CMD_End:
				/**
					ʵ�����ָ����ݽ���ָ�����FPGA״̬�Ѽ�ת���̡߳��Ͽ�socket���ӽ�����
					ָ���룺2bit��
					1������FPGA״̬�Ѽ�ת���߳�
					2������һЩ�ڴ���ͷţ��豸�رղ���
					3���������ȷ��
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