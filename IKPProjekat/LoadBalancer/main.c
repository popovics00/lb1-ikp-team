#pragma warning( disable : 4996)
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

#include "communication.h"
#include "structs.h"
#include "RingBuffer.h"

#pragma comment(lib,"ws2_32.lib")
#define PORT 5059	//port na kom slusa LB
#define INITIAL_CAPACITY_BUFFER 1000

//GLOBAL VARS
Queue* primaryQueue = NULL;
Queue* tempQueue = NULL;
Queue* secondaryQueue = NULL;

int main()
{
	/*
			    	LBfromBrRecieverThread
	--------------------------------------------------------
	Kreira socket, osluskuje i prima poruke u novom THREAD-u
	Obradjuje ih i cuva ih u QUEUE
	*/
	SOCKET serverSocket = SetListenSocket(PORT);

	primaryQueue = CreateQueue(INITIAL_CAPACITY_BUFFER);
	DWORD komunikacijaSaCuvanjemURedId;
	HANDLE komunikacijaSaCuvanjemURed = CreateThread(NULL,
		0,
		WorkWithSockets,
		&serverSocket,
		0,
		&komunikacijaSaCuvanjemURedId
	);
	
	
	
	
	
	//CISTO DA SE NE UGASI PROGRAM
	int temp = 0;
	while (1) {
		scanf("%d", &temp);
		printf("%d", temp);
	}

	//zatvaranje niti
	CloseHandle(komunikacijaSaCuvanjemURed);
	return 0;
}
