#include <stdio.h>
#include <winsock2.h>
#include <time.h>

#define BUFFER_SIZE 1024 * 64 // 64K

u_short SERVER_PORT = 2642; // 侦听端口
char serverName[64] = "Server name: lab6-webserver";
WORD wVersionRequested;
WSADATA wsaData;
int ret, nLeft, length;
SOCKET sListen, sHttp, sServer;		 // 连接套接字
struct sockaddr_in saServer, saHttp; // 地址信息

DWORD WINAPI handlemsg(LPVOID param)
{
	SOCKET tcp = *((int *)param);
	SOCKADDR_IN HttpInfo = {0};
	int addrsize = sizeof(HttpInfo);
	getpeername(tcp, (struct sockaddr *)&HttpInfo, &addrsize);
	char *IP = inet_ntoa(HttpInfo.sin_addr);
	int port = HttpInfo.sin_port;
	while (1)
	{
		char recvPacket[BUFFER_SIZE], method[BUFFER_SIZE], url[BUFFER_SIZE], version[BUFFER_SIZE], filename[BUFFER_SIZE];
		int recvState = recv(tcp, recvPacket, BUFFER_SIZE, 0);
		if (recvState > 0)
		{
			printf("\n<recv packet> \n%s\n", recvPacket);
			sscanf(recvPacket, "%s %s %s", method, url, version);
			printf("<method> %s\n", method);
			// GET
			if (stricmp(method, "GET") == 0)
			{
				char filetype[20];
				strcpy(filename, "../info");
				printf("<url> %s\n", url);
				if (strstr(url, "html"))
				{
					strcat(filename, "/html");
					strcpy(filetype, "text/html");
				}
				else if (strstr(url, "txt"))
				{
					strcat(filename, "/txt");
					strcpy(filetype, "text/plain");
				}
				else if (strstr(url, "img"))
				{
					strcpy(filetype, "image/jpeg");
				}
				strcat(filename, url);
				printf("<filename> %s\n", filename);
				FILE *fp = fopen(filename, "rb");
				if (fp == NULL)
				{
					printf("<error> file not found\n");
					char *msg = "<html><body>File Not Found!</body></html>";
					char buf[BUFFER_SIZE] = "HTTP/1.1 404 NOT Found\r\n";
					strcat(buf, "Connection: keep-alive\r\n");
					strcat(buf, "Content-Type: text/html\r\n");
					strcat(buf, "Content-Length: ");
					char len[5];
					itoa(strlen(msg), len, 10);
					strcat(buf, len);
					strcat(buf, "\r\n\r\n");
					strcat(buf, msg);
					if (!send(tcp, buf, strlen(buf), 0))
					{
						printf("<error> send error\n");
					}
				}
				else
				{
					printf("<log> file found\n");
					fseek(fp, 0, SEEK_SET);
					fseek(fp, 0, SEEK_END);
					long filelen = ftell(fp);
					int headlen;
					char len[5];
					_ltoa(filelen, len, 10);
					char *filecontent;
					filecontent = (char *)malloc(filelen);
					rewind(fp);
					fread(filecontent, sizeof(char), filelen, fp);
					if (filecontent == NULL)
					{
						printf("<error> file get error\n");
						break;
					}
					char buf[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\n";
					strcat(buf, "Connection: keep-alive\r\n");
					strcat(buf, "Content-Length: ");
					strcat(buf, len);
					strcat(buf, "\r\n");
					strcat(buf, "Server: csr_http1.1\n");
					if (strstr(url, "img"))
						strcat(buf, "Accept-Ranges: bytes\r\n");
					strcat(buf, "Content-Type: ");
					strcat(buf, filetype);
					strcat(buf, "\r\n\r\n");
					headlen = strlen(buf);
					memcpy(buf + strlen(buf), filecontent, filelen);
					if (send(tcp, buf, headlen + filelen, 0))
					{
						printf("<log> send success\n");
					}
				}
				fclose(fp);
			}
			else if (stricmp(method, "POST") == 0)
			{
				printf("<url> %s\n", url);
				if (stricmp(url, "/dopost") != 0)
				{
					char *msg = "<html><body>File Not Found</body></html>";
					char buf[BUFFER_SIZE] = "HTTP/1.1 404 NOT Found!\r\n";
					strcat(buf, "Connection: keep-alive\r\n");
					strcat(buf, "Content-Type: text/html\r\n");
					strcat(buf, "Content-Length: ");
					char len[5];
					itoa(strlen(msg), len, 10);
					strcat(buf, len);
					strcat(buf, "\r\n\r\n");
					strcat(buf, msg);
					if (!send(tcp, buf, strlen(buf), 0))
					{
						printf("<error> send error\n");
					}
				}
				else
				{
					char buf[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\n";
					strcat(buf, "Connection: keep-alive\r\n");
					strcat(buf, "Server: csr_http1.1\r\n");
					strcat(buf, "Content-Type: text/html\r\n");
					strcat(buf, "Content-Length: ");
					char *loginfo = strstr(recvPacket, "login=");
					int andplace = 0;
					while (loginfo[andplace] != '\0')
					{
						if (loginfo[andplace] == '&')
							break;
						andplace++;
					}
					char login[20], pass[20];
					strncpy(login, loginfo + strlen("login="), andplace - strlen("login="));
					strncpy(pass, loginfo + andplace + strlen("&pass="), strlen(loginfo) - andplace);
					printf("<login> %s\n", login);
					printf("<pass> %s\n", pass);
					if (strcmp(login, "3200102642") == 0 && strcmp(pass, "2642") == 0)
					{
						char *msg = "<html><body>Login Success!</body></html>";
						char len[5];
						itoa(strlen(msg), len, 10);
						strcat(buf, len);
						strcat(buf, "\r\n\r\n");
						strcat(buf, msg);
						if (!send(tcp, buf, strlen(buf), 0))
						{
							printf("<error> send error\n");
						}
					}
					else
					{
						char *msg = "<html><body>Login Failed!</body></html>";
						char len[5];
						itoa(strlen(msg), len, 10);
						strcat(buf, len);
						strcat(buf, "\r\n\r\n");
						strcat(buf, msg);
						if (!send(tcp, buf, strlen(buf), 0))
						{
							printf("<error> send error\n");
						}
					}
				}
			}
		}
	}
	printf("<log> Close the Sever\n");
	closesocket(tcp);
	return 0;
}

int main()
{
	// WinSock初始化：
	wVersionRequested = MAKEWORD(2, 2); // 希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return 0;
	}
	// 确认WinSock DLL支持版本2.2：
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return 0;
	}

	// 创建socket，使用TCP协议：
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return 0;
	}
	else
		printf("<INFO> initialize success\n");

	// 构建本地地址信息：
	saServer.sin_family = AF_INET;					   // 地址家族
	saServer.sin_port = htons(SERVER_PORT);			   // 注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // 使用INADDR_ANY指示任意地址

	// 绑定：
	ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("bind() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen); // 关闭套接字
		WSACleanup();
		return 0;
	}
	else
		printf("<INFO> bind success\n");

	// 侦听连接请求：
	ret = listen(sListen, 20);
	if (ret == SOCKET_ERROR)
	{
		printf("listen() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen); // 关闭套接字
		WSACleanup();
		return 0;
	}
	else
		printf("<INFO> listen success\n");

	printf("<INFO> server is ready\n");
	printf("<INFO> quit: Ctrl+c\n");
	// 阻塞等待接受客户端连接：
	while (1)
	{
		length = sizeof(saHttp);
		sServer = accept(sListen, (struct sockaddr *)&saHttp, &length);
		if (sServer == INVALID_SOCKET)
		{
			printf("accept() failed! code:%d\n", WSAGetLastError());
			closesocket(sListen); // 关闭套接字
			WSACleanup();
			return 0;
		}
		HANDLE subThread;
		subThread = CreateThread(NULL, 0, handlemsg, &sServer, 0, 0);
	}
	closesocket(sListen); // 关闭套接字
	WSACleanup();
}