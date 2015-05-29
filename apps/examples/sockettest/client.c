#include <nuttx/config.h>
#include <nuttx/clock.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <apps/netutils/dnsclient.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int sockettest_main(int argc, char *argv[])
#endif
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	char buffer[256];

	printf("%s, in\n", __func__);

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		return -1;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("ERROR opening socket, sockfd: %d\n", sockfd);
		return -1;
	}

	printf("%s, 1, host: %s, port: %d\n", __func__, argv[1], portno);

	/*
	 * debug by Librae
	 */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1 * USEC_PER_DSEC;
	n = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof(struct timeval));
	if (n != 0) {
		printf("ERROR setsockopt, n: %d\n", n);
		goto errout;
	}
	printf("%s, 2, tv_usec: %d\n", __func__, tv.tv_usec);

	n = dns_gethostip(argv[1], &serv_addr.sin_addr.s_addr);
	if (n != 0) {
		fprintf(stderr,"ERROR, no such host, n: %d\n", n);
		goto errout;
	}
	printf("%s, 3, s_addr: 0x%08x\n", __func__, serv_addr.sin_addr.s_addr);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	n = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if (n < 0)
	{
		printf("ERROR connecting, n: %d\n", n);
		goto errout;
	}
	printf("%s, 4\n", __func__);

	bzero(buffer,256);
	strcpy(buffer, "hello from sockettest");
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0)
		printf("ERROR writing to socket, n: %d\n", n);
	printf("%s, 5\n", __func__);

	bzero(buffer,256);
	n = read(sockfd,buffer,255);
	if (n < 0)
		printf("ERROR reading from socket, n: %d\n", n);
	printf("%s\n",buffer);

errout:
	close(sockfd);
	printf("%s, out\n", __func__);
	return 0;
}
