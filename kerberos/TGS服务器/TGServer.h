#ifndef TGSERVER_H_
#define TGSERVER_H_

/************************************************
	在Windows下运行的TGS服务器端程序头文件
	功能：
		1 完成TGS端的Kerberos访问控制流程
	todo：
		1 完善注释和代码规范
************************************************/

#pragma comment(lib, "WS2_32.lib")

#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<vector>
#include<string>
#include<ctime>
#include<windows.h>
#include"../General.h"
#include"../Cryptography.h"
#define BufferSize 128
using namespace std;

string Version = "0.0.1-dev";

class TGS
{
private:
	int time_delay = 2000;				//有效延迟时间，秒
	int srvPort = 1501;					//服务端口
	int Error = 0;
	int QueueLength = 5;
	char SendBuffer[BufferSize];		//发送缓冲区
	char RecvBuffer[BufferSize];		//接收缓冲区
	long int time_now;					//实时时间
	WORD wsaVersion = MAKEWORD(2, 2);	//wsa版本
	SOCKET TGSrvSocket;					//服务器端socket
	SOCKET ClientSocket;				//用户端socket
	time_t Time;


public:
	TGS(){}

	//启动初始化
	bool StartUp()
	{
		WSADATA TGSwsa;

		Error = WSAStartup(wsaVersion, &TGSwsa);
		if (Error != 0)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		TGSrvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (TGSrvSocket == INVALID_SOCKET)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		sockaddr_in TGSrv_Info;
		TGSrv_Info.sin_family = AF_INET;
		TGSrv_Info.sin_port = htons(srvPort);
		TGSrv_Info.sin_addr.s_addr = htonl(INADDR_ANY);

		Error = ::bind(TGSrvSocket, (LPSOCKADDR)&TGSrv_Info, sizeof(TGSrv_Info));
		if (Error == SOCKET_ERROR)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		return true;
	}

	bool StandBy()
	{
		Error = listen(TGSrvSocket, QueueLength);
		if (Error < 0)
		{
			cout << "Error" << WSAGetLastError() << endl;
			exit(-1);
		}

		sockaddr_in ClientSockAddr;
		int ClientSocketAddr_Length = sizeof(ClientSockAddr);
		ClientSocket = accept(TGSrvSocket, (SOCKADDR*)&ClientSockAddr, &ClientSocketAddr_Length);
		if (ClientSocket == INVALID_SOCKET)
		{
			cout << "Error!" << WSAGetLastError() << endl;
			exit(-1);
		}

		return true;
	}

	//主运行函数
	void Run()
	{
		string msgTemp;
		string ClientID;
		string ClientMAC;

		StartUp();

		while (true)
		{
			StandBy();

			// 1 接收客户端发送的ServiceID/TGT
			string ServiceID, TimeStamp, TGT_temp;
			SocketRecv(ClientSocket, RecvBuffer,BufferSize);
			msgTemp = RecvBuffer;
			memset(RecvBuffer, 0, sizeof(RecvBuffer));

			// 2 获取ServiceID和TGT
			const char plus = '+';
			vector<string> Msg_SrvID_TGT;
			StringSplit(Msg_SrvID_TGT, msgTemp);
			msgTemp.clear();
			if (Msg_SrvID_TGT.size() != 2)
			{	//分割失败
				cout << "Error!" << endl;
				exit(-1);
			}
			ServiceID = Msg_SrvID_TGT[0];
			msgTemp = Msg_SrvID_TGT[1];
			Msg_SrvID_TGT.clear();
			/* 查找对应serviceID的密钥*/
			TGT_temp = decrypt(getPasswdof("TGS"), msgTemp); //解密TGT

			// 2.5 接收验证器{ClientID,TimeStamp}
			string Authenticator;
			Error = recv(ClientSocket, RecvBuffer, sizeof(RecvBuffer), 0);
			if (Error < 0)
			{
				cout << "Error!" << WSAGetLastError() << endl;
				exit(-1);
			}
			msgTemp = RecvBuffer;
			Authenticator = msgTemp;
			msgTemp.clear();
			memset(RecvBuffer, 0, sizeof(RecvBuffer));

			// 3 验证TGT
			string TGS_SessionKey;
			vector<string> TicketGT;
			StringSplit(TicketGT, TGT_temp);
			if (TicketGT.size() != 4)
			{
				cout << "error!" << endl;
				exit(-1);
			}
			/*检查时效性*/
			time_now = int(getTime());
			if (stoi(TicketGT[2], nullptr, 0) < time_now)
			{
				cout << "TGT过期" << endl;
				exit(-1);
			}
			TGS_SessionKey = TicketGT[0];
			ClientID = TicketGT[1];
			ClientMAC = TicketGT[3];

			// 4 检查验证器
			vector<string> Auth_temp;
			Authenticator = decrypt(TGS_SessionKey, Authenticator);
			StringSplit(Auth_temp, Authenticator);
			if (Auth_temp.size() != 2)
			{
				cout << "Check ID Error!" << endl;
				exit(-1);
			}
			time_now = int(getTime());
			long int ts = stoi(Auth_temp[1], nullptr, 0) + time_delay;
			if (time_now > ts)
			{
				cout << "Authenticator Timeout!" << endl;
				exit(-1);
			}
			if (strcmp(Auth_temp[0].c_str(), ClientID.c_str()))
			{
				cout << "Authenticator Client Mismatch!" << endl;
				exit(-1);
			}

			// 5 签发Client_to_Server_Ticket
			string CtoSrvTicket;
			string Client_Server_SessionKey = genSessionKey(); //会话密钥生成!_!
			CtoSrvTicket = Client_Server_SessionKey + "+" + ClientID + "+" + to_string(getTime() + 30) + "+" + ClientMAC;
			msgTemp = encrypt(getPasswdof(ServiceID.c_str()), CtoSrvTicket);
			StringToChar(SendBuffer, msgTemp);
			msgTemp.clear();
			SocketSend(ClientSocket, SendBuffer);
			memset(SendBuffer, 0, sizeof(SendBuffer));
			Sleep(1000);

			// 5.5 发送Client/Server SessionKey
			msgTemp = encrypt(TGS_SessionKey, Client_Server_SessionKey);
			StringToChar(SendBuffer, msgTemp);
			SocketSend(ClientSocket, SendBuffer);
			msgTemp.clear();
			memset(SendBuffer, 0, sizeof(SendBuffer));

			// 6 完成
			closesocket(ClientSocket);
		}
		WSACleanup();
	}


};



#endif // !TGSERVER_H_