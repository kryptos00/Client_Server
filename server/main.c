// server code for UDP socket programming
#include <winsock2.h>
#include <errno.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define IP_PROTOCOL 0
#define PORT_NO 15050
#define NET_BUF_SIZE 32
#define cipherKey 'S'
#define sendrecvflag 0
#define nofile "File Not Found!"

// function to clear buffer
void clearBuf(char *b)
{
	int i;
	for (i = 0; i < NET_BUF_SIZE; i++)
		b[i] = '\0';
}

// function to encrypt
char Cipher(char ch)
{
	//return ch ^ cipherKey;
	if ((ch >= 97 && ch <= 122) || (ch >= 65 && ch <= 90))
    {
        if ((ch > 109 && ch <= 122) || (ch > 77 && ch <= 90))
            ch = ch - 13;
        else
            ch += 13;
    }

    return ch;
}

// function sending file
int sendFile(FILE *fp, char *buf, int s)
{
	int i, len;
	if (fp == NULL)
	{
		strcpy(buf, nofile);
		len = strlen(nofile);
		buf[len] = EOF;
		for (i = 0; i <= len; i++)
			buf[i] = Cipher(buf[i]);
		return 1;
	}

	char ch, ch2;
	for (i = 0; i < s; i++)
	{
		ch = fgetc(fp);
		ch2 = Cipher(ch);
		buf[i] = ch2;
		if (ch == EOF)
			return 1;
	}
	return 0;
}

// driver code
int main()
{
	int sockfd, nBytes;
	struct sockaddr_in addr_con;
	int addrlen = sizeof(addr_con);
	addr_con.sin_family = AF_INET;
	addr_con.sin_port = htons(PORT_NO);
	addr_con.sin_addr.s_addr = INADDR_ANY;
	char net_buf[NET_BUF_SIZE];
	FILE *fp;

	WSADATA wsa;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 0), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	// socket()
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);

	if (sockfd < 0)
	{
		//printf( "Error opening file: %s\n", strerror( GetLastError() ));
		printf("Could not create socket : %d", WSAGetLastError());
		//printf("\nfile descriptor not received!!\n");
		//auto error_code{ ::GetLastError() };
		//throw std::system_error( error_code, std::system_category(),"Failed to retrieve error message string.");
	}
	else
		printf("\nfile descriptor %d received\n", sockfd);

	// bind()
	if (bind(sockfd, (struct sockaddr *)&addr_con, sizeof(addr_con)) == 0)
		printf("\nSuccessfully binded!\n");
	else
		printf("\nBinding Failed!\n");

	while (1)
	{
		printf("\nWaiting for file name...\n");

		// receive file name
		clearBuf(net_buf);

		nBytes = recvfrom(sockfd, net_buf,
						  NET_BUF_SIZE, sendrecvflag,
						  (struct sockaddr *)&addr_con, &addrlen);

		fp = fopen(net_buf, "r");
		printf("\nFile Name Received: %s\n", net_buf);
		if (fp == NULL)
			printf("\nFile open failed!\n");
		else
			printf("\nFile Successfully opened!\n");

		while (1)
		{

			// process
			if (sendFile(fp, net_buf, NET_BUF_SIZE))
			{
				sendto(sockfd, net_buf, NET_BUF_SIZE,
					   sendrecvflag,
					   (struct sockaddr *)&addr_con, addrlen);
				break;
			}

			// send
			sendto(sockfd, net_buf, NET_BUF_SIZE,
				   sendrecvflag,
				   (struct sockaddr *)&addr_con, addrlen);
			clearBuf(net_buf);
		}
		if (fp != NULL)
			fclose(fp);
	}
	return 0;
}
