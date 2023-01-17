#define BUFLEN 1024	//max duzina buffera
#define INITIAL_CAPACITY_BUFFER 1000
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "structs.h"
#include "RingBuffer.h"
#include "communicationWorker.h"

Node* headMetersList = NULL; // lista metera
Queue* primaryQueue = NULL; //ring buffer zahtev
int globalIdClient = 0;
unsigned long nonBlockingMode = 1;
unsigned long BlockingMode = 0;
CRITICAL_SECTION cs;

void inicijalizacijeReda() {
	if (primaryQueue == NULL)
		primaryQueue = CreateQueue(1000);
}

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




/*
PrijemDaljihPoruka - funkcija koju pokreće WorkWithSockets u zasebnim tredovima koje čuvamo u meteru. Svrha funkcije
je da očekuje poruke od registrovanih metera i da pravi račune koje ce čuvati u RingBuffer.
*/

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
				Node* temp = headMetersList;
				while (temp != NULL) {
					if (socket == temp->meter->acceptedSocket) {
						recvbuf[iResult] = '\0';
						printf("\n\nPrimljen izvestaj od:\nThread id = %d.\Meter id: %d.\nPort: %d\nIP adresa: %s.\nDuzina poruke: %d \nPotrosnja za ovaj mesec: %s \n", GetCurrentThreadId(), temp->meter->id, temp->meter->port, temp->meter->ipAdr, iResult, recvbuf);
						int number = atoi(recvbuf);
						//printf("\nStatus prosli mesec -> Stats ovaj mesec: %d->%d\n\n", temp->meter->lastMonth, number);

						//kreiramo racun i cuvamo ga u ring buffer
						Racun* r = (Racun*)malloc(sizeof(Racun));
						r->meterId = temp->meter->id;
						r->stanjeTrenutno = number;
						r->stanjeStaro = temp->meter->lastMonth;
						SetajTrenutnoStanje(&headMetersList, temp->meter->id, number);
						enqueue(&primaryQueue, r);
						ispisiRacune(primaryQueue);
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
	deleteNode(&headMetersList, socket);
	globalIdClient--;
	return 0;
}



/*
WorkWithSockets - funkcija koja prati dešavanja na prosleđenom soketu, selektujemo ga i pratimo odgovore.
Ukoliko pristigne poruka zahteva za konekciju povratna vrednost selekcije bude > 0 i znamo da je pristigao neki zahtev.
Pravimo novi meter i cuvamo ga na kraju liste.
*/

DWORD WINAPI WorkWithSockets(void* vargp) {
	SOCKET serverSocket = *(SOCKET*)vargp;

	do
	{
		struct sockaddr_in adresa;
		int addrlen = sizeof(adresa);
		int iResult;

		struct timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		FD_SET set;
		FD_ZERO(&set);
		FD_SET(serverSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		SetNonblocking(&serverSocket);
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
		else {		//pristigao zahtev za konekciju na soket za metere, 
			Meter* newMeter = (Meter*)malloc(sizeof(Meter));
			newMeter->acceptedSocket = accept(serverSocket, (struct sockaddr*)&adresa, &addrlen);
			if (newMeter->acceptedSocket == INVALID_SOCKET)
			{
				printf("\naccept failed with error: %d", WSAGetLastError());
				closesocket(serverSocket);
				WSACleanup();
				return 1;
			}

			//cuvamo sve informacije o meteru i zapocinjemo thread za prijem novih racuna tog klijenta
			char clientip[20];
			strcpy(clientip, inet_ntoa(adresa.sin_addr));
			DWORD threadId;
			newMeter->id = globalIdClient++;
			newMeter->acceptedSocket = newMeter->acceptedSocket;
			newMeter->ipAdr = clientip;
			newMeter->port = adresa.sin_port;
			newMeter->lastMonth = 0;
			newMeter->debt = 0;
			newMeter->thread = CreateThread(NULL,
				0,
				PrijemDaljihPoruka,
				&newMeter->acceptedSocket,
				0,
				&threadId
			);
			printf(
				"\n---------------------------\n\tBrojilo[%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\tje prihvaceno\t\n---------------------------\n"
				, newMeter->id, newMeter->id, newMeter->ipAdr, newMeter->port, threadId
			);

			AddAtEnd(&headMetersList, newMeter);	//dodaj meter na kraj liste


		}
	} while (1);
}

/*
ObradaRacuna - pozivamo je za slobodnog workera koje ga zauzme tako što property zauzet postavi na true 
i posalje mu zadatak u formatu METER ID/STANJE STARO/STANJE NOVO. Zatim primi odgovor od workera i pozove 
funkciju UvecajDug koja izmeni meter i oslobodi workera. Tu se ta nit, završava do slede upotrebe tog workera.
*/

DWORD WINAPI ObradaRacuna(void* vargp) {
	Worker* worker = (Worker*)vargp;
	EnterCriticalSection(&cs);
	Racun temp = dequeue(&primaryQueue);
	LeaveCriticalSection(&cs);
	char str[BUFLEN];
	//formatiranje poruke za slanje METER ID/STANJE STARO/STANJE NOVO
	sprintf(str, "%d/%d/%d",temp.meterId, temp.stanjeStaro, temp.stanjeTrenutno);

	if (temp.meterId != -1)	//ako racun postoji
	{
		worker->zauzet = true;
		DWORD threadId;
		printf("\nObrada racuna pre slanja:\n\tRacun za slanje je (meterid/starostanje/trenutno stanje): %s", str);
		int iResult = send(worker->acceptedSocket, str, (int)strlen(str), 0);
		Sleep(2000);
		iResult = recv(worker->acceptedSocket, str, BUFLEN, 0);
		str[iResult] = '\0';
		printf("\nObrada racuna dobijen izvestaj:\n\tRacun primljen je (meterid/racun): %s", str);
		if (iResult>1) {
			char* ptr = strtok(str, "/");	//parsiramo poruku
			int id = atoi(ptr);
			ptr = strtok(NULL, "/");
			int novoDugovanje = atoi(ptr);
			//printf("\n \t id %d dug novi %d", id, novoDugovanje);
			UvecajDug(&headMetersList,id, novoDugovanje);	//menjamo metera
			worker->zauzet = false;
		}
		return;
	}
	worker = NULL;
}

/*
SlanjeSoketima - funkcija koju pozivamo u main niti, njena uloga je da proverava broj workera i broj računa koji cekaju na računanje.
Ukoliko ih imamo vršimo preuzimanje računa iz buffera  i slanje prvom slobodnom workeru. Worker u svojoj strukturi dobija thread
koji poziva funkciju ObradaRačuna(worker).
*/

void SlanjeSoketima() {
	int brojWorkera = 0;
	Worker* worker = NULL;
	while (true) {
		Sleep(1000);
		brojWorkera = IzbrojWorkere(headWorkerList);
		printf("\nBroj workera: %d \tBroj neobradjenih racuna: %d",brojWorkera, primaryQueue->size);
		if (primaryQueue->size > 0 && brojWorkera > 0) {
			//EnterCriticalSection(&cs);
			worker = VratiSlobodnogWorkera(headWorkerList);
			//LeaveCriticalSection(&cs);
			//ispisiRacune(primaryQueue);
			if (worker == NULL) {
				printf("\nNema slobodnog workera");
				continue;
			}
			else {
				DWORD threadId;
				worker->thread = CreateThread(NULL,
					0,
					ObradaRacuna,
					worker,
					0,
					&threadId
				);
			}
		}
		Sleep(1000);
	}
	return 0;
}

