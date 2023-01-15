#pragma warning( disable : 4996)
#pragma warning( disable : 4700)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "meterList.h"
#include "structs.h"
#include "communication.h"
#include "RingBuffer.h"

#pragma comment(lib,"ws2_32.lib")
#define PORT 5059	//port na kom slusa LB
#define PORTWorker 5079	//port na kom slusa LB
#define INITIAL_CAPACITY_BUFFER 1000


int main()
{
	inicijalizacijeReda();
	//pravljenje soketa
	SOCKET serverSocket = SetListenSocket(PORT);
	SOCKET workerSocket = SetListenSocket(PORTWorker);

	//osluskuje na 1 soketu i registruje nove metere u red metera
	DWORD komunikacijaSaCuvanjemURedId;
	HANDLE komunikacijaSaCuvanjemURed = CreateThread(NULL,
		0,
		WorkWithSockets,
		&serverSocket,
		0,
		&komunikacijaSaCuvanjemURedId
	);

	//rad sa registracijom workera i cuvanjem u red i poruka u ring buffer
	DWORD komunikacijaSaCuvanjemWorkeraURedId;
	HANDLE komunikacijaSaCuvanjemWorkeraURed = CreateThread(NULL,
		0,
		WorkWithSocketsWorker,
		&workerSocket,
		0,
		&komunikacijaSaCuvanjemWorkeraURedId
	);

	//kupimo poruke i rasporedjujemo ih
	SlanjeSoketima();


	//CISTO DA SE NE UGASI PROGRAM
	int temp = 0;
	while (1) {
		scanf("%d", &temp);
		printf("%d", temp);
		if (temp == 0)
			break;
	}

	//zatvaranje niti i prazni zauzeto
	FreeList(headMetersList); // ciscenje liste metera
	CloseHandle(komunikacijaSaCuvanjemURed);
	CloseHandle(komunikacijaSaCuvanjemWorkeraURed);
	return 0;
}
