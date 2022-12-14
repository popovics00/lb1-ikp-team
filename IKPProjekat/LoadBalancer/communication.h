#define BUFLEN 1024	//max duzina buffera

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "structs.h"

Node* headClients;
int globalIdWorker = 0;
int globalIdClient = 0;
unsigned long nonBlockingMode = 1;
unsigned long BlockingMode = 0;


void SetNonblocking(SOCKET* socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}
}
void SetBlocking(SOCKET* socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &BlockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}
}

bool InitializeWindowsSockets()
{
	WSADATA wsa;
	//Inicijalizacija winsocka
	printf("\nInicijalizacija windows soketa...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return false;
	}
	return true;
}


SOCKET SetListenSocket(int port) {
	SOCKET serverSocket;

	if (InitializeWindowsSockets() == false)
	{
		printf("\n[GRESKA] Lose inicijalizovan soket > %d", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	printf("\n[+] Socket inicijalizovan uspesno.");

	//Pravljenje soketa
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("\n[GRESKA] Neuspesno kreiran soket : %d", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	printf("\nSocket uspesno kreiran.");

	//inicijalizacija adrese
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	//Bindovanje adresa za soket
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("\n[GRESKA] Bind neuspesno, greska: %d", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	printf("\nBindovanje adrese za soket uspesno.");
	SetNonblocking(&serverSocket);
	int iResult = listen(serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	return serverSocket;
}

DWORD WINAPI PrijemDaljihPoruka(void* vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	int iResult = 0;
	struct timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	struct sockaddr_in adresaKlijenta;
	int addrlen = sizeof(adresaKlijenta);
	int numberRecv = 0;
	SetBlocking(&socket);
	while (true) {
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(socket, &set);

		char recvbuf[BUFLEN];
		iResult = select(0, &set, &set, NULL, &timeVal);

		if (FD_ISSET(socket, &set)) {
			if (iResult == SOCKET_ERROR) {
				printf("\nioctlsocket failed with error: %d", WSAGetLastError());
				break;
			}
			iResult = recv(socket, recvbuf, BUFLEN, 0);

			int currentLength = 0;

			if (iResult > 0)
			{
				//Node* temp = headClients;
				//while (temp != NULL) {
				//	if (socket == temp->client->acceptedSocket) {
				//		//EnterCriticalSection(&CriticalSectionForOutput);
				//printf("\n\nRecv client message:\nThread id = %d.\nClient id: %d.\nPort: %d\nIP address: %s.\nLength message: %d\n\n", GetCurrentThreadId(), temp->client->id, temp->client->port, temp->client->ipAdr, iResult);
				//		//LeaveCriticalSection(&CriticalSectionForOutput);
				//		break;
				//	}
				//	temp = temp->next;
				//}
				recvbuf[iResult] = '\0';
				printf("%s", recvbuf);
				continue;

			}
			else if (iResult == 0)
			{
				printf("\nKonekcija sa klijentom zatvorena.");
				closesocket(socket);
				break;
			}
			else
			{
				printf("\nrecv failed with error: %d", WSAGetLastError());
				closesocket(socket);
				break;
			}
		}
	}
	//deleteNode(&headClients, socket);
	return 0;
}


DWORD WINAPI WorkWithSockets(void* vargp) {
	SOCKET serverSocket = *(SOCKET*)vargp;

	do
	{
		struct sockaddr_in adresa;
		int addrlen = sizeof(adresa);
		int iResult;
		//timeval
		struct timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;
		//fdset
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(serverSocket, &set);
		//FD_SET(workerSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		//printf("\n\t(iResult > %d)", iResult);
		SetNonblocking(&serverSocket);
		if (iResult == SOCKET_ERROR) {	//doslo je do greske
			//EnterCriticalSection(&CriticalSectionForOutput);
			printf("\nselect failed with error: %d", WSAGetLastError());
			//LeaveCriticalSection(&CriticalSectionForOutput);
		}
		else if (iResult == 0) {	//nista se nije desilo, idemo dalje
			if (_kbhit()) {
				break;
			}
			//EnterCriticalSection(&CriticalSectionForOutput);
			printf("\nCekanje klijenta...");
			//LeaveCriticalSection(&CriticalSectionForOutput);
			continue;
		}
		else { //pristigao zahtev za konekciju na soket za metere
			Client* newMeter = (Client*)malloc(sizeof(Client));
			newMeter->acceptedSocket = accept(serverSocket, (struct sockaddr*)&adresa, &addrlen);
			if (newMeter->acceptedSocket == INVALID_SOCKET)
			{
				//EnterCriticalSection(&CriticalSectionForOutput);
				printf("\naccept failed with error: %d", WSAGetLastError());
				//LeaveCriticalSection(&CriticalSectionForOutput);
				closesocket(serverSocket);
				WSACleanup();
				return 1;
			}
			char clientip[20];
			strcpy(clientip, inet_ntoa(adresa.sin_addr));
			DWORD threadId;
			newMeter->id = globalIdClient++;
			newMeter->acceptedSocket = newMeter->acceptedSocket;
			newMeter->ipAdr = clientip;
			newMeter->port = adresa.sin_port;
			newMeter->thread = CreateThread(NULL,
				0,
				PrijemDaljihPoruka,
				&newMeter->acceptedSocket,
				0,
				&threadId
			);
			//EnterCriticalSection(&CriticalSectionForOutput);
			printf(
				"\n---------------------------\n\tBrojilo[%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\tje prihvaceno\t\n---------------------------\n"
				, newMeter->id,
				newMeter->id,
				newMeter->ipAdr,
				newMeter->port,
				threadId
			);
			//LeaveCriticalSection(&CriticalSectionForOutput);
		}
	} while (1);
}