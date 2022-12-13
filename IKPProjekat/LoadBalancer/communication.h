#define BUFLEN 512	//max duzina buffera

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
	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
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
	
	return serverSocket;
}

DWORD WINAPI AddToQueue(SOCKET serverSocket) {

	struct sockaddr_in adresaPrimaoca;
	int adr_len, recv_len;
	char buf[BUFLEN];
	adr_len = sizeof(adresaPrimaoca);
	while (1)
	{
		printf("\nCekanje poruke...");
		fflush(stdout);

		//ciscenje buffera i zauzimanje prostora
		memset(buf, '\0', BUFLEN);

		//prijem poruke
		if ((recv_len = recvfrom(serverSocket, buf, BUFLEN, 0, (struct sockaddr*)&adresaPrimaoca, &adr_len)) == SOCKET_ERROR)
		{
			printf("recvfrom() greska prilikom prijema: %d", WSAGetLastError());
			return -2;
		}

		printf("Pristigla poruka od %s:%d\n", inet_ntoa(adresaPrimaoca.sin_addr), ntohs(adresaPrimaoca.sin_port));
		printf("Poruka: %s\n", buf);

		//AKCIJA KAD SE PORUKA PREUZME
		strcpy(buf, "test");
		if (sendto(serverSocket, buf, sizeof(buf), 0, (struct sockaddr*)&adresaPrimaoca, adr_len) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			return -1;
		}
	}
	closesocket(serverSocket);
	WSACleanup();
}