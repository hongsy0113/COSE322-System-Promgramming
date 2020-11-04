#include "stdio.h"
#include <stdlib.h>
#include "string.h"
#include "time.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "pthread.h"
#include "unistd.h"
#include "sys/time.h"
#include "netinet/in.h"
#include "arpa/inet.h"

#define BUF_LEN 128
#define INPUT_LEN 128

void at_checking(char * buf, int len);
void client_creator(void *num);
int atsign_counting(const char * const buf, size_t len);
struct timeval tv;
int counting(short *port_num, int size);

int main()
{

	// get the number of clients and each port numbers
	char *input = (char *)malloc(INPUT_LEN);

	INPUT_CODE: // return position
	memset((void *)input, 0, INPUT_LEN);
	fgets(input,INPUT_LEN ,stdin);

	// parse the given string into individual port info
	char *token = strtok(input, " ");
	int client_len = atoi(token); // the number of clients
	if(client_len > 10) // when it is over 10	
		goto INPUT_CODE;

	short *port_num= (short*)malloc(sizeof(short) * client_len);// array of port numbers for each client	
	int i = 0;  // iteration num for input port_num
	token = strtok(NULL, " ");
 	for(; token != NULL &&  i < client_len ; i++ )
	{
	port_num[i] = atoi(token);  // convert string to short number
	token = strtok(NULL, " ");
	}
	// access the same port number over 5
	if ( counting(port_num, i) == -1)
		goto INPUT_CODE;
	// when the number of ports is under what you typed
	else if( i != client_len) 
		printf("the input port number is below what you've typed earlier!\n"); 	 

	int j = 0; // iteration num for thread_create
	pthread_t client_threads[i];
	for( ; j< i;j ++)
	{		
		pthread_create(&client_threads[j], NULL, (void *)&client_creator,  (void *)&port_num[j]);
	} 
	
	for( j=0; j< i; j++)
	{
		pthread_join(client_threads[j], NULL);

	}
	return 0;
}

void client_creator(void *num) // thread function
{
	short *port_num = (short *) num;
	int client_fd;
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // get client file descriptor while checking error
	{
	printf("Client : Can't open stream socket\n");
	exit(0);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family =AF_INET;
	server_addr.sin_port = htons(*port_num);
	server_addr.sin_addr.s_addr= inet_addr("192.168.56.101");
	
	if( connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) // connect server and client while checking error
	{
	printf("Connection failed!\n");
	exit(1);
	}

	char fname[20] = {0,};  // set empty array to make record-file name
	sprintf(fname, "%d-%d.txt", *port_num, client_fd); // file name. For exmaple, 1111-4
	char buff[BUF_LEN];
	FILE *ffp= fopen(fname, "w");  //open new file to record recieved msg
	int at_num = 0; // counter for the number of @ 
	int hour; int min; int sec; int msec;
	while(1)
	{
		int read_n = read(client_fd, buff, BUF_LEN); // save the number of bytes read to use later
		at_num += atsign_counting(buff, read_n);
		gettimeofday(&tv, NULL);
		hour = tv.tv_sec / 3600; // second to hour
		min = (tv.tv_sec - (hour * 3600)) / 60; // After cutting off hour, convert second to minute
		sec = tv.tv_sec % 60;
		msec = tv.tv_usec / 1000; // usec to msec	
		if( at_num >= 5)
		{
			at_checking(buff, read_n); // put '\0' right behind the fifth '@'
			fprintf(ffp, "%02d:%02d:%02d.%03d|%d|%d|%s\n", hour, min, sec, msec, *port_num, read_n, buff);
			break;
		}
		
		fprintf(ffp, "%02d:%02d:%02d.%03d|%d|%d|%s\n", hour, min, sec, msec, *port_num, read_n, buff);
}
	

	fclose(ffp); //file close
	if(close(client_fd) == -1) // close client socket
	{
		printf("close failed!\n");
	}	
	printf("%s Completed!\n", fname);
	
}

void at_checking(char * buf, int len) // insert '/0' right behind the fifth '@' in buffer
{

	int i;
	int n = 0;
	for (i = 0; i< len; i++)
	{
		if(buf[i] =='@')
			n++;
		if(n ==5)
		 {
			buf[i+1] = '\0';
			break;

			}
	}

	
}


int atsign_counting(const char * const buf, size_t len) // count the number of '@' in buffer
{

	int i;
	int n=0;
	for ( i =0; i < len ; i++)
	{	
		if(buf[i] =='@')
			n++;
	}
	return n;
}

int counting (short *port_num, int size) // for checking error when it comes to excessive accesses on the same port (over 5)
{
	short port_check; 

	int i=0, j = 0;
	int count = 0;        

	for (;i< size; i++){
		port_check = port_num[i];
		for(j = 0; j< size; j++)
		{	if( port_check == port_num[j])
				count ++;
		}
		if( count >5 )
		{
			printf("too many accesses on the same port!, please try again.\n");
			return -1;
		}
		count= 0 ;
	}
	return 0;
}
