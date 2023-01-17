#define BUFLEN 1024	//max duzina buffera
#define INITIAL_CAPACITY_BUFFER 1000

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "structs.h"
#include "RingBuffer.h"
#include "workerList.h"

CRITICAL_SECTION cs;
NodeW* headWorkerList = NULL; // lista metera
int globalIdWorker = 0;
unsigned long nonBlockingModeWorker = 1;
unsigned long BlockingModeWorker = 0;

void SetNonblockingWorker(SOCKET* socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &nonBlockingModeWorker);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}
}
void SetBlockingWorker(SOCKET* socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &BlockingModeWorker);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}
}
/*
WorkWithSocketsWorker - funkcija koja osluškuje na drugom soketu i čeka zahteve za registraciju novih workera koje kreira i čuva u novi red.
*/

DWORD WINAPI WorkWithSocketsWorker(void* vargp) {
	SOCKET serverWorkerSocket = *(SOCKET*)vargp;

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
		FD_SET(serverWorkerSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		SetNonblockingWorker(&serverWorkerSocket);
		if (iResult == SOCKET_ERROR) {	//doslo je do greske
			printf("\nselect failed with error: %d", WSAGetLastError());
		}
		else if (iResult == 0) {	//nista se nije desilo, idemo dalje
			if (_kbhit()) {
				break;
			}
			//printf("\nCekanje klijenta...");
			continue;
		}
		else { //pristigao zahtev za konekciju na soket za metere
			Worker* newWorker = (Worker*)malloc(sizeof(Worker));
			newWorker->acceptedSocket = accept(serverWorkerSocket, (struct sockaddr*)&adresa, &addrlen);
			if (newWorker->acceptedSocket == INVALID_SOCKET)
			{
				printf("\naccept failed with error: %d", WSAGetLastError());
				closesocket(serverWorkerSocket);
				WSACleanup();
				return 1;
			}
			char clientip[20];
			strcpy(clientip, inet_ntoa(adresa.sin_addr));
			DWORD threadId;
			newWorker->id = globalIdWorker++;
			newWorker->acceptedSocket = newWorker->acceptedSocket;
			newWorker->ipAdr = clientip;
			newWorker->port = adresa.sin_port;
			newWorker->zauzet = false;
			printf(
				"\n---------------------------\n\tWorker [%d]\nid: %d\nip Address: %s\nport: %d\n\tje prihvaceno\t\n---------------------------\n"
				, newWorker->id, newWorker->id, newWorker->ipAdr, newWorker->port
			);

			AddAtEndWorker(&headWorkerList, newWorker);	//dodavanje na kraj liste
		}
	} while (1);
}
