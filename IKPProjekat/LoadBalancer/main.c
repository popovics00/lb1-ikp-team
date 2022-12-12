#pragma warning( disable : 4996)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>

#include "declerations.h"

#pragma comment(lib,"ws2_32.lib")

#define BUFLEN 512	//max duzina buffera
#define PORT 5059	//port na kom slusa LB

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	slen = sizeof(si_other);


	WSADATA wsa;
	//Inicijalizacija winsocka
	printf("\nInicijalizacija windows soketa...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("\n[GRESKA] Lose inicijalizovan soket > %d", WSAGetLastError());
		return 0;
	}
	printf("\nSocket inicijalizovan uspesno.");

	//Pravljenje soketa
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("\n[GRESKA] Neuspesno kreiran soket : %d", WSAGetLastError());
		return 0;
	}
	printf("\nSocket uspesno kreiran.");

	//inicijalizacija adrese
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bindovanje adresa za soket
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("\n[GRESKA] Bind neuspesno, greska: %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("\nBindovanje adrese za soket uspesno.");




	//sada komuniciramo
	while (1)
	{
		printf("\nCekanje poruke...");
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
