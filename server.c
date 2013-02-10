#include <time.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>

#define MAXLINE 255
#define LISTENQ 5

void main(int argc, char **argv){
	int listenfd,connfd,n;
	socklen_t len;
	struct sockaddr_in servaddr, cliaddr;
	char buf[MAXLINE];
	char buf2[1024];
	char buf3[1024] = "[200700223 jingkoo zzang]\n";
	time_t ticks;

	listenfd = socket(AF_INET,SOCK_STREAM,0);  

	bzero(&servaddr, sizeof(servaddr));  
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5069);

	
	bind(listenfd,(struct sockaddr *)&servaddr, sizeof(servaddr));


	listen(listenfd, LISTENQ);




	len = sizeof(cliaddr);
	connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len); 


	printf("connection from %s, port %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr, buf,sizeof(buf) ), ntohs(cliaddr.sin_port));
	ticks = time(NULL);
	snprintf(buf,sizeof(buf),"%.24s\r\n",ctime(&ticks));
	write(connfd, buf, strlen(buf));

	while(1){
		FILE *fp; 
		fp = fopen("echoprogram.txt","a");  
		recv(connfd, buf2, sizeof(buf2),0); 
		bzero(buf,MAXLINE);
		ticks = time(NULL); 
		snprintf(buf,sizeof(buf),"%.24s",ctime(&ticks));
		strcat(buf," "); 
		strcat(buf,buf2);
		fprintf(fp,"%s",buf);
		strcat(buf,buf3); 
		write(1,buf,strlen(buf)); 
		write(connfd, buf, strlen(buf)); 
		fclose(fp);
	}
	close(connfd);
}
