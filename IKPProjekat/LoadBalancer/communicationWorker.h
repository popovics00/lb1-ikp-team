#define BUFLEN 1024	//max duzina buffera
#define INITIAL_CAPACITY_BUFFER 1000

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "structs.h"
#include "RingBuffer.h"
#include "workerList.h"

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

DWORD WINAPI DaljiRadWorker(void* vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	int iResult = 0;
	struct timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	struct sockaddr_in adresaKlijenta;
	int addrlen = sizeof(adresaKlijenta);
	int numberRecv = 0;
	SetBlockingWorker(&socket);
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
				NodeW* temp = headWorkerList;
				while (temp != NULL) {
					if (socket == temp->worker->acceptedSocket) {
						recvbuf[iResult] = '\0';
						printf("\n\nPrimljen izvestaj od:\nThread id = %d.\Worker id: %d.\nPort: %d\nIP adresa: %s.\nDuzina poruke: %d \nPotrosnja za ovaj mesec: %s \n", GetCurrentThreadId(), temp->worker->id, temp->worker->port, temp->worker->ipAdr, iResult, recvbuf);
						
						//int number = atoi(recvbuf);
						//printf("\nStatus prosli mesec -> Stats ovaj mesec: %d->%d\n\n", temp->worker->lastMonth, number);
						//printf("\nDug je sada -> %d dinara\n\n", temp->worker->debt);
						//UvecajDug(&headWorkerList, temp->meter->id, number);

						//printf("\n\nLista metera\n");
						//IspisiListu(headWorkerList);
						//Racun* r = (Racun*)malloc(sizeof(Racun));
						//r->meterId = temp->meter->id;
						//r->stanjeTrenutno = number;
						//push(&primaryQueue);
						//enqueue(&primaryQueue, r);
						break;
					}
					temp = temp->next;
				}
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
	deleteNodeWorker(&headWorkerList, socket);
	globalIdWorker--;
	return 0;
}


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
			newWorker->thread = CreateThread(NULL,
				0,
				DaljiRadWorker,
				&newWorker->acceptedSocket,
				0,
				&threadId
			);
			printf(
				"\n---------------------------\n\tWorker [%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\tje prihvaceno\t\n---------------------------\n"
				, newWorker->id,
				newWorker->id,
				newWorker->ipAdr,
				newWorker->port,
				threadId
			);
			AddAtEndWorker(&headWorkerList, newWorker);
		}
	} while (1);
}
