#pragma warning( disable : 4996)

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"	//ip adresa UDP servera
#define BUFLEN 512	//max duzina buffera
#define PORT 5059	//port na kom server slusa

int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

	//inicijalizacija winsock
	printf("\nInicijalizacija Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Greska. Error kod: %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Inicijalizovan uspesno.\n");

	//pravljenje socketa
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() ima gresku : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//podesavanje adresne strukture za slanje
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	//pocetak komunikacije
	while (1)
	{
		printf("Unesite poruku: ");
		gets(message);

		//slanje poruke
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() greska sa kodom: %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//prijem odgovora od LB
		
		//ciscenje buffera
		memset(buf, '\0', BUFLEN);
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() greska prilikom odgovora od servera: %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Odgovor od LB: %s\n",buf);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}