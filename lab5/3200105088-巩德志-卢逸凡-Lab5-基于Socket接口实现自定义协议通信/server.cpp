#include <stdio.h>
#include <winsock2.h>
#include <time.h>

//�������ݰ���ʽ
struct text
{
	int type = 0; // 0: sent message, 1: get time, 2: get name, 3: get list
	int target = 0; // �յ���Ϣ�Ŀͻ��˴���
	char data[256];
};

struct ClientInfo {
	char *IP;
	int port;
	int active;
	SOCKET tcp;
} ClientList[10];

u_short SERVER_PORT = 2642; //�����˿�
char serverName[64] = "********server name: MyServer";
WORD wVersionRequested;
WSADATA wsaData;
int ret, nLeft, length;
SOCKET sListen, sClient, sServer; //�����׽���
struct sockaddr_in saServer, saClient;//��ַ��Ϣ
struct text message;
char *ptr = (char *)&message;
char addr[20];
BOOL fSuccess = TRUE;

void transMessage(SOCKET tcp, struct text *rec)
{
	char *s;
	memcpy(s+8, rec->data, sizeof(rec->data));
	send(ClientList[rec->target].tcp, s, 256, 0);
	printf("<INFO> message has been sent to client\n");
}

void getTime(SOCKET tcp)
{
	time_t t;
	time(&t);
	struct tm* tt;
	tt = localtime(&t);
	char s[256];
	sprintf(s, "********time: %d.%d.%d %d:%d:%d", (1900 + tt->tm_year), (1 + tt->tm_mon),
		tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
	send(tcp, s, 256, 0);
	printf("<INFO> time has been sent to client\n");
}

void getName(SOCKET tcp)
{
	char s[256];
	memcpy(s, serverName, sizeof(serverName));
	send(tcp, s, 256, 0);
	printf("<INFO> name has been sent to client\n");
}

void getList(SOCKET tcp)
{
	char s[256];
	strcpy(s, "********client list:\n");
	for (int i = 0; i < 10; i++) {
		if (ClientList[i].active) {
			char *tmp;
			itoa(i, tmp, 10);
			strcat(s, tmp);
			strcat(s, "\n");
		}
	}
	send(tcp, s, 256, 0);
	printf("<INFO> client list has been sent to client\n");
}

//�������߳�ѭ��
DWORD WINAPI handleClient(LPVOID param)
{
	SOCKET tcp = *((int *)param);
	SOCKADDR_IN ClientInfo = {0};
	int addrsize = sizeof(ClientInfo);
	getpeername(tcp, (struct sockaddr*)&ClientInfo, &addrsize);
	char *IP = inet_ntoa(ClientInfo.sin_addr);
	int port = ClientInfo.sin_port;
	for (int i = 1; i < 10; i++) {
		if (!ClientList[i].active) {
			ClientList[i].active = 1;
			ClientList[i].tcp = tcp;
			ClientList[i].IP = IP;
			ClientList[i].port = port;
			break;
		}
	}
	while (1) {
		char recvPackage[256];
		int recvState = recv(tcp, recvPackage, 256, 0);
		struct text* rec = (struct text*)recvPackage;
		if (recvState > 0) {
			printf("<INFO> package received\n");
			switch (rec->type)
			{
				case 0:
					transMessage(tcp, rec);
					break;
				case 1:
					getTime(tcp);
					//for(int i=0;i<100;i++) getTime(tcp);
					break;
				case 2:
					getName(tcp);
					break;
				case 3:
					getList(tcp);
					break;
				default:
					break;
			}
		}
		else {
			printf("<INFO> client connection closed \n");
			break;
		}
	}
	int closeflag = 1;
	for (int i = 1; i < 10; i++) {
		if (ClientList[i].tcp == tcp) {
			ClientList[i].active = 0;
			break;
		}
		if(ClientList[i].active == 1) closeflag = 0;
	}
	if(closeflag) {
		closesocket(tcp);
		free(param);
		return 0;
	}
}

int main()
{
	//WinSock��ʼ����
	wVersionRequested = MAKEWORD(2, 2);//ϣ��ʹ�õ�WinSock DLL�İ汾
	ret = WSAStartup( wVersionRequested, &wsaData );
	if (ret != 0)
	{
		printf("<ERROR> WSAStartup() failed!\n");
		return 0;
	}
	//ȷ��WinSock DLL֧�ְ汾2.2��
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
	{
		WSACleanup();
		printf("<ERROR> Invalid Winsock version!\n");
		return 0;
	}

	//����socket��ʹ��TCPЭ�飺
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("<ERROR> socket() failed!\n");
		return 0;
	}
	else printf("<INFO> initialize success\n");
	
	//�������ص�ַ��Ϣ��
	saServer.sin_family = AF_INET;//��ַ����
	saServer.sin_port = htons(SERVER_PORT);//ע��ת��Ϊ�����ֽ���
	saServer.sin_addr.S_un.S_addr = inet_addr("10.192.209.8");  //�����IP��ַ

	//�󶨣�
	ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("<ERROR> bind() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);//�ر��׽���
		WSACleanup();
		return 0;
	}
	else printf("<INFO> bind success\n");

	//������������
	ret = listen(sListen, 5);
	if (ret == SOCKET_ERROR)
	{
		printf("<ERROR> listen() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);//�ر��׽���
		WSACleanup();
		return 0;
	}
	else printf("<INFO> listen success\n");

	printf("<INFO> server is ready\n");
	//�����ȴ����ܿͻ������ӣ�
	while (1) {
		length = sizeof(saClient);
		sServer = accept(sListen, (struct sockaddr *)&saClient, &length);
		if(sServer == INVALID_SOCKET)
		{
			printf("<ERROR> accept() failed! code:%d\n", WSAGetLastError());
			closesocket(sListen);//�ر��׽���
			WSACleanup();
			return 0;
		}
		HANDLE subThread;
		subThread = CreateThread(NULL, 0, handleClient, &sServer, 0, 0);
	}
	closesocket(sListen);//�ر��׽���
	closesocket(sServer);
	WSACleanup();
}