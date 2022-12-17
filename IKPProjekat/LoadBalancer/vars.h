#pragma once
#include <stdbool.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <windows.h>

#include "structs.h"
#include "RingBuffer.h"
#include "communication.h"
#include "meterList.h"


#pragma comment(lib,"ws2_32.lib")
#define BUFLEN 1024	//max duzina buffera
#define PORT 5059	//port na kom slusa LB
#define INITIAL_CAPACITY_BUFFER 1000

Queue* primaryQueue = NULL;
Queue* tempQueue = NULL;
Queue* secondaryQueue = NULL;
Queue* reorQueue = NULL;
