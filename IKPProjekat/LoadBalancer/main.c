#pragma warning( disable : 4996)

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512	//Max length of buffer
#define PORT 5059	//The port on which to listen for incoming data

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Inicijalizacija winsocka
	printf("\nInicijalizacija Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Greka. Kod greske: %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Inicijalizovan uspesno.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Nemoguce kreirati soket : %d", WSAGetLastError());
	}
	printf("Socket uspesno kreiran.\n");

	//inicijalizacija adrese
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind neuspesno, greska: %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bindovanje uspesno");

	while (1)
	{
		printf("Cekanje poruke...");
		fflush(stdout);

		//ciscenje buffera i zauzimanje prostora
		memset(buf, '\0', BUFLEN);

		//prijem poruke
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() greska prilikom prijema: %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		printf("Pristigla poruka od %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Poruka: %s\n", buf);

		//odgovaramo klijentu
		strcpy(buf,"test");
		if (sendto(s, buf, sizeof(buf), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}