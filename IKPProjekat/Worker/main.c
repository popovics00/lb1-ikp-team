#pragma warning( disable : 4996)

#include<stdio.h>
#include<winsock2.h>
#include <stdbool.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"	//ip adresa UDP servera
#define BUFLEN 512	//max duzina buffera
#define PORT 5079	//port na kom server slusa

unsigned long nonBlockingMode = 1;
unsigned long blockingMode = 0;

void SetNonblocking(SOCKET* socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("\nioctlsocket failed with error: %d", WSAGetLastError());
	}
}
void SetBlocking(SOCKET* socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &blockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("\nioctlsocket failed with error: %d", WSAGetLastError());
	}
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("\nWSAStartup failed with error: %d", WSAGetLastError());
		return false;
	}
	return true;
}


SOCKET SetConnectedSocket(u_short port) {
	SOCKET connectSocket = INVALID_SOCKET;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("\nsocket failed with error: %ld", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(port);

	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("\nGreska prilikom konektovanja na server. %ld", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	SetNonblocking(&connectSocket);

	return connectSocket;
}

int main(void)
{
	int prosliMesec = 0;
	int ovajMesec = 0;
	char buf[BUFLEN];
	char message[BUFLEN];
	int iResult = 0;
	SOCKET s = SetConnectedSocket(PORT);
	if (s == 1) {
		printf("\nStisni enter za izlaz...");
		getchar();
		return 0;
	}
	while (1) {
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(s, &set);

		struct timeval timeVal;
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		iResult = select(0, NULL, &set, NULL, &timeVal);

		//printf("\n(IResult > %d)", iResult);
		if (iResult == SOCKET_ERROR) {	//error
			printf("\nGreska prilikom selekcije soketa: %d", WSAGetLastError());
			return -1;
		}
		else if (iResult == 0) {
			printf("\nCekamo odgovor...");
			Sleep(100);
			return -1;
		}
		else if (FD_ISSET(s, &set)) { // send
			char slanjePoruka[] = "0";
			char recvbuf[BUFLEN];
			printf("\nCekamo poruku od servera >> ");
			iResult = send(s, slanjePoruka, (int)strlen(slanjePoruka), 0);
			printf("Poruka uspesno poslata! (IResult: %d)", iResult);
			
			SetBlocking(&s);
			while (1)
			{
				iResult = recv(s, recvbuf, BUFLEN, 0);
				recvbuf[iResult] = '\0';

				printf("\n%s", recvbuf);
				char* ptr = strtok(recvbuf, "/");
				char* id = ptr;
				ptr = strtok(NULL, "/");
				int pocetak = atoi(ptr);
				ptr = strtok(NULL, "/");
				int kraj = atoi(ptr);

				int racun = (kraj - pocetak) * 117;
				printf("\n\tRacun je (id/pocetak/kraj/racun): %s %d %d %d", id,pocetak,kraj,racun);
				sprintf(recvbuf,"%s/%d",id,racun);
				int iResult = send(s, recvbuf, (int)strlen(recvbuf), 0);
				Sleep(3000);
			}
			break;
		}
	}
	closesocket(s);
	WSACleanup();
	printf("\nBrojilo je ugaseno...");
	getchar();
	return 0;
}


