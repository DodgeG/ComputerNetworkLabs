#include <stdio.h>
#include <winsock2.h>

//�������ݰ���ʽ
struct text
{
	int type = 0; // 0: sent message, 1: get time, 2: get name, 3: get list
	int target = 0; // �յ���Ϣ�Ŀͻ��˴���
	char data[256];
};

u_short SERVER_PORT ; //�����˿�
WORD wVersionRequested;
WSADATA wsaData;
int ret, nLeft, length;
SOCKET sListen, sClient, sServer; //�����׽���
struct sockaddr_in saServer, saClient;//��ַ��Ϣ
struct text message;
char *ptr = (char *)&message;
char addr[20];
BOOL fSuccess = TRUE;

//�������ݵ����߳�
DWORD WINAPI acceptMessage(LPVOID param) {
	SOCKET tcp = *((int*)param);
	while (1) {
		char recvPackage[256];
		int recvState = recv(tcp, recvPackage, 256, 0);
		struct text* rec = (struct text*)recvPackage;
		if (recvState > 0) printf("%s\n", rec->data);
		else break;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	//WinSock��ʼ����
	wVersionRequested = MAKEWORD(2, 2);//ϣ��ʹ�õ�WinSock DLL�İ汾
	ret = WSAStartup( wVersionRequested, &wsaData );
	if (ret != 0) {
		printf("<ERROR> WSAStartup() failed!\n");
		return 0;
	}
	//ȷ��WinSock DLL֧�ְ汾2.2��
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) {
		WSACleanup();
		printf("<ERROR> Invalid Winsock version!\n");
		return 0;
	}

	//����socket��ʹ��TCPЭ�飺
	sClient = socket(AF_INET, SOCK_STREAM, 0);
	if(sClient == INVALID_SOCKET) {
		WSACleanup();
		printf("<ERROR> socket() failed!\n");
		return 0;
	}
	
	//������������ַ��Ϣ��
	while(1) {
		int num;
		printf("<MENU> input function code:\n\t0: connect\n\t1: disconnect\n\t2: get time\n\t3: get server name\n\t4: get client list\n\t5: send message\n\t6: quit\n\n");
		scanf("%d", &num);
		switch( num ) {
			case 0: {
				printf("<MENU> input address:\n");
				scanf("%s", addr);
				printf("<MENU> input port number:\n");
				scanf("%d",&SERVER_PORT);
				saServer.sin_family = AF_INET;//��ַ����
				saServer.sin_port = htons(SERVER_PORT);//ע��ת��Ϊ�����ֽ���
				saServer.sin_addr.S_un.S_addr = inet_addr(addr);
				//���ӷ�������
				ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
				if (ret == SOCKET_ERROR) {
					printf("<ERROR> connect() failed!\n");
					closesocket(sClient);//�ر��׽���
					WSACleanup();
					system("pause");
					break;
				}
				else printf("<INFO> connect successed\n");
				//���߳�ѭ������receive()
				HANDLE subThread;
				subThread =  CreateThread(NULL, 0, acceptMessage, &sClient, 0, 0);
				break;
			}
			case 1: {
				closesocket(sClient);//�ر��׽���
				WSACleanup();
				printf("<INFO> disconnect successed\n");
				break;
			}
			case 2: {
				//for(int i=0;i<100;i++){
				message.type = 1; // get time
				ret = send(sClient, (char *)&message, sizeof(message), 0);
				if(ret == SOCKET_ERROR) printf("<ERROR> get time failed!\n");
				//}
				break;
			}
			case 3: {
				message.type = 2; // get name
				ret = send(sClient, (char *)&message, sizeof(message), 0);
				if(ret == SOCKET_ERROR) printf("<ERROR> get name failed!\n");
				break;
			}
			case 4: {
				message.type = 3; // get list
				ret = send(sClient, (char *)&message, sizeof(message), 0);
				if(ret == SOCKET_ERROR) printf("<ERROR> get client list failed!\n");
				break;
			}
			case 5: {
				message.type = 0;
				printf("<MENU> input client number:\n");
				scanf("%d", &message.target);
				printf("<MENU> input message:\n");
				scanf("%s", message.data);
				ret = send(sClient, (char *)&message, sizeof(message), 0);
				if(ret == SOCKET_ERROR) printf("<ERROR> send() failed!\n");
				else printf("<INFO> message sent\n");
				break;
			}
			case 6: {
				closesocket(sClient);//�ر��׽���
				WSACleanup();
				return 0;
			}
			default:
				printf("<ERROR> invalid function code!");
				break;
		}
	}
	closesocket(sClient);//�ر��׽���
	WSACleanup();
	return 0;
}