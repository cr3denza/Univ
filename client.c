#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAXLINE 255

void main(int argc, char **argv){
	int sockfd, n;
	char recvline[MAXLINE+1];
	char buf[1024]={0};
	struct sockaddr_in servaddr;

	if(argc != 2)
		fprintf(stderr, "usage : client <ipaddress>");

	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
		fprintf(stderr, "socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5069);

	if(inet_pton(AF_INET,argv[1], &servaddr.sin_addr) <= 0)
		fprintf(stderr, "inet_pton error for %s", argv[1]);
	if(n = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0){
		n = read(sockfd, recvline, MAXLINE);
		recvline[n]=0;
		if(fputs(recvline, stdout)==EOF)
			fprintf(stderr, "fputs error");

		while(1){
			bzero(buf,MAXLINE);
			printf("message : "); 
			if(fgets(buf, sizeof(buf), stdin) == NULL ){
				close(sockfd); 
				exit(1);
			}
			if(send(sockfd, buf, sizeof(buf), 0) < 0 ){
				perror("send");
				close(sockfd);
				exit(1);
			}

			recv(sockfd, buf, sizeof(buf),0); 
			write(1,buf,strlen(buf)); 

		}
	}
	if(n<0)
		fprintf(stderr, "read error");

	close(sockfd);
	exit(0);
}
