#ifndef SERVICESERVER_H_
#define SERVICESERVER_H_

/***********************************************
	在Windows下运行的应用服务器端访问控制头文件
	功能：
		1 实现与客户端之间的Kerberos访问控制流程
		2 协商后续服务的会话密钥
		3 处理错误和异常消息
	todo：
		1 代码优化和注释完善
************************************************/

#pragma comment(lib,"WS2_32.lib")

#include<iostream>
#include<string>
#include<vector>
#include<winsock2.h>
#include<WS2tcpip.h>
#include"../Cryptography.h"
#include"../General.h"
#define BufferSize 256
using namespace std;

class ServiceServer
{
private:
	int time_now;						//现在时间
	int time_delay = 30;				//最大延迟时间
	size_t Error;						//接收返回值
	int SSrvPort = 1505;				//SS端口
	int Service_Port = 1403;			//提供服务的端口
	int QueueLength = 3;				//客户端队列长度
	SOCKET ClientSocket;				//客户端socket
	SOCKET SSrvSocket;					//服务器端socket
	char SendBuffer[BufferSize];		//发送缓冲区
	char RecvBuffer[BufferSize];		//接收缓冲区
	string CtoSrvSessionKey;			//服务器与客户端的会话密钥
	WORD wsaVersion = MAKEWORD(2, 2);	//WSA版本号
	time_t Time;


public:
	ServiceServer()
	{}

	//启动并初始化
	bool StartUp()
	{
		WSADATA SSrvWsa;

		Error = WSAStartup(wsaVersion, &SSrvWsa);
		if (Error != 0)
		{
			cout << "ERROR!" << WSAGetLastError() << endl;
			exit(-1);
		}

		SSrvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SSrvSocket == INVALID_SOCKET)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		sockaddr_in SSrv_Info;
		SSrv_Info.sin_family = AF_INET;
		SSrv_Info.sin_port = htons(SSrvPort);
		SSrv_Info.sin_addr.s_addr = htonl(INADDR_ANY);
		int SSrv_info_length = sizeof(SSrv_Info);

		Error = ::bind(SSrvSocket, (sockaddr *)&SSrv_Info, SSrv_info_length);  //通过::bind()的方式可以不调用std里的bind函数
		if (Error == SOCKET_ERROR)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		return true;
	}

	bool StandBy()
	{
		Error = listen(SSrvSocket, QueueLength);
		if (Error != 0)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		sockaddr_in ClientSockAddr;
		int ClientSockAddrLength = sizeof(ClientSockAddr);
		ClientSocket = accept(SSrvSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);
		if (ClientSocket == INVALID_SOCKET)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		return true;
	}

	void MessageExchange()
	{
		SOCKET sTOcSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SSrvSocket == INVALID_SOCKET)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}
		
		sockaddr_in SSrv_Info;
		SSrv_Info.sin_family = AF_INET;
		SSrv_Info.sin_port = htons(Service_Port);
		SSrv_Info.sin_addr.s_addr = htonl(INADDR_ANY);
		int SSrv_info_length = sizeof(SSrv_Info);

		Error = ::bind(SSrvSocket, (sockaddr*)&SSrv_Info, SSrv_info_length);  //通过::bind()的方式可以不调用std里的bind函数
		if (Error == SOCKET_ERROR)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		Error = listen(SSrvSocket, QueueLength);
		if (Error != 0)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		sockaddr_in ClientSockAddress;
		int ClientSockAddrLength = sizeof(ClientSockAddress);
		SOCKET ClientSocket;				//客户端socket
		ClientSocket = accept(SSrvSocket, (SOCKADDR*)&ClientSockAddress, &ClientSockAddrLength);
		if (ClientSocket == INVALID_SOCKET)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		SocketRecv(ClientSocket, RecvBuffer, BufferSize);
		cout << RecvBuffer << endl;
		closesocket(ClientSocket);
	}

	void Run()
	{
		string msgTemp, ClientID, TimeStamp, ClientMAC;
		const char plus = '+';

		StartUp();

		while (true)
		{
			StandBy();

			// 1 接收客户端发送的Client_to_Server Ticket并解析
			string CtoSrvTicket_temp;
			SocketRecv(ClientSocket, RecvBuffer, BufferSize);
			if (Error < 0)
			{
				cout << "ERROR!" << WSAGetLastError() << endl;
				exit(-1);
			}
			msgTemp = RecvBuffer;
			memset(RecvBuffer, 0, sizeof(RecvBuffer));
			/*使用自己的密钥解密*/
			CtoSrvTicket_temp = decrypt(getPasswdof("SS"), msgTemp);

			// 2 检查Client_to_Server Ticket
			vector<string> CtoSrvTicket;
			StringSplit(CtoSrvTicket, CtoSrvTicket_temp);
			if (CtoSrvTicket.size() != 4)
			{
				cout << "ERROR!" << endl;
				exit(-1);
			}
			/*检查时效*/
			time_now = int(getTime());
			if (stoi(CtoSrvTicket[2], nullptr, 0) < time_now)
			{
				cout << "Timeout!" << endl;
				exit(-1);
			}
			CtoSrvSessionKey = CtoSrvTicket[0];
			ClientID = CtoSrvTicket[1];
			ClientMAC = CtoSrvTicket[3];

			// 3 接收验证器{ClientID,TimeStamp}
			SocketRecv(ClientSocket, RecvBuffer, BufferSize);
			msgTemp = RecvBuffer;
			memset(RecvBuffer, 0, sizeof(RecvBuffer));

			// 4 处理验证器
			string Auth_temp;
			Auth_temp = decrypt(CtoSrvSessionKey, msgTemp);
			vector<string> Authenticator;
			StringSplit(Authenticator, Auth_temp);
			if (Authenticator.size() != 2)
			{
				cout << "Error!" << endl;
				exit(-1);
			}

			// 4.5 进行验证
			if (strcmp(Authenticator[0].c_str(), ClientID.c_str()))
			{	//id不同
				cout << "ERROR!" << endl;
				exit(-1);
			}
			time_now = int(getTime());
			long int ts = stoi(Authenticator[1], nullptr, 0) + time_delay;
			if (time_now > ts)
			{
				cout << "TIMEOUT!" << endl;
				exit(-1);
			}

			// 5 返回时间戳
			TimeStamp = encrypt(CtoSrvSessionKey, Authenticator[1]);
			StringToChar(SendBuffer, TimeStamp);
			SocketSend(ClientSocket, SendBuffer);

			// 6 验证通过
			closesocket(ClientSocket);
		}
		MessageExchange();
		WSACleanup();
	}

};


#endif // !SERVICESERVER_H_