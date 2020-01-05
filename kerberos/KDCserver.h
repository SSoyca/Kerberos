#ifndef KDCSERVER_H_
#define KDCSERVER_H_

/***********************************************
	此版本的KDC服务器在Windows环境下运行，
	需求：
	1 登记并管理用户口令和用户组
	2 登记并管理下属服务器密钥
	3 完整的事件日志记录
	4 签发访问凭证 - done
	5 验证用户身份 - done
	6 可被客户端检测
	todo:
	1 完成数据库相关功能实现
	2 对错误消息的处理
************************************************/

#pragma comment(lib, "ws2_32.lib")

#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<string>
#include<cstring>
#include<errno.h>
#include<ctime>
#include<iostream>
#include"Cryptography.h"
//#include"DatabaseOperation.h"
#define BufferSize 256	//缓冲区长度
using namespace std;

string versionID = "0.0.1-dev";
/*

Kerberos中的KDC服务器

*/

class KDCsrv
{
	
private:
	int srvPort = 1500;			    	 //服务端口
	int timeout = 1500;					//超时上限
	int QueueLenth = 5;					//监听队列长度
	int RunTimeError = 0;				//接受错误返回
	int time_now;						//现在时间
	char sendBUFF[BufferSize];			//发送缓冲区
	char recvBUFF[BufferSize];			//接收缓冲区
	WORD wVersion = MAKEWORD(2, 2);		//wsa版本号
	SOCKET AuthsrvSock;					//服务器socket
	time_t Time;
	
	//启动并初始化
	bool Startup()
	{
		WSADATA AuthWSA;

		RunTimeError = WSAStartup(wVersion, &AuthWSA);  //启动wsa
		if (RunTimeError != 0)
		{
			cout << "wsaStartup ERROR!";//wsastartup错误
			exit(-1);
		}

		AuthsrvSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //初始化socket对象
		if (AuthsrvSock == INVALID_SOCKET)
		{
			cout << "Socket init ERROR!";//启动socket报错退出
			exit(-1);
		}

		sockaddr_in AuthSrv_info;
		AuthSrv_info.sin_family = AF_INET;
		AuthSrv_info.sin_port = htons(srvPort);
		AuthSrv_info.sin_addr.s_addr = htonl(INADDR_ANY);
		//inet_pton(AF_INET, "192.168.137.102", &AuthSrv_info.sin_addr.S_un.S_addr);

		RunTimeError = ::bind(AuthsrvSock, (LPSOCKADDR)&AuthSrv_info, sizeof(AuthSrv_info));  //绑定端口到指定socket
		if (RunTimeError == SOCKET_ERROR)
		{
			cout << "bind error! " << WSAGetLastError() << endl;;//启动报错退出
			exit(-1);
		}

		return true;
	}

	//开始运行并等待客户端连接
	SOCKET Standby()
	{
		RunTimeError = listen(this->AuthsrvSock, QueueLenth);  //监听socket
		if (RunTimeError != 0)
		{
			cout << "error! " << WSAGetLastError() << endl;//监听函数报错退出
			exit(-1);
		}

		sockaddr_in ClientSockAddr;  //socket地址类
		int clientsockAddrLenth = sizeof(ClientSockAddr);
		SOCKET ClientSock = accept(this->AuthsrvSock, (SOCKADDR*)&ClientSockAddr, &clientsockAddrLenth);
		if (ClientSock == INVALID_SOCKET)
		{
			cout << "error! " << WSAGetLastError() << endl;//接受函数报错退出
			exit(-1);
		}
		
		
		return ClientSock;
	}
public:
	KDCsrv() 
	{
		memset(&sendBUFF, 0, sizeof(sendBUFF));
	}

	//数据库相关，用户注册
	bool Resgiter()
	{
		//数据库连接不成功

		return true;
	}

	//KDC服务器的运行主函数
	void Run()
	{
		int ClientAddr_length;		//用户地址类长度
		char ClientIP_temp[32];		//存放客户端IP
		SOCKET ClientSocket;		//客户端SOCKET对象
		sockaddr_in ClientAddr;		//存放客户端地址信息
		string msgTmp;				//处理缓冲
		string userName;			//用户名
		string ClientID;			//用户id
		string ClientIP;			//用户地址
		string TGT, TicketTimeout;	//TGT，ticket有效期
		string passwd, TGSSessionKey;//密码，tgs会话密钥

		Startup();

		//循环此流程
		while (true)
		{
			ClientSocket = Standby();//响应客户端连接请求

			// 1 接收客户端发来的用户名并查询用户
			RunTimeError = recv(ClientSocket, recvBUFF, sizeof(recvBUFF), 0);
			if (RunTimeError < 0)
			{
				cout << "recv Error!"; //接收函数错误退出
				exit(-1);
			}
			msgTmp = recvBUFF;
			memset(recvBUFF, 0, sizeof(recvBUFF));
			/*
			sql!
			*/
			//!__!查询数据库内容，注意防范注入攻击

			// 2 生成并发送用户ID和TGS
			//debug-
			userName = "zcy";
			passwd = "abcdef";
			ClientID = "1710300822";
			//-debug
			TicketTimeout = to_string(timeout + getTime());
			ClientAddr_length = sizeof(ClientAddr);
			getpeername(ClientSocket, (sockaddr*)&ClientAddr, &ClientAddr_length);
			inet_ntop(AF_INET, &(ClientAddr.sin_addr), ClientIP_temp, 32);
			ClientIP = ClientIP_temp;
			//passwd = sql(userID)//sql 中读出密码并hash
			passwd = genHash(passwd);
			TGSSessionKey = genSessionKey();
			msgTmp = userName + "+" + TGSSessionKey + "+" + ClientID;
			StringToChar(sendBUFF, msgTmp);
			SocketSend(ClientSocket, sendBUFF);
			memset(sendBUFF, 0, sizeof(sendBUFF));

			// 3 生成TGT，获取TGS密钥并使用密钥加密TGT，并发送
			msgTmp = TGSSessionKey + "+" + ClientID + "+" + TicketTimeout + "+" + ClientIP;
			TGT = encrypt(getPasswdof("TGS"), msgTmp);
			StringToChar(sendBUFF, TGT);
			SocketSend(ClientSocket, sendBUFF);
			memset(sendBUFF, 0, sizeof(sendBUFF));

			// 4 断开连接，等待下一用户请求
			closesocket(ClientSocket);
			userName.clear(); passwd.clear(); TGSSessionKey.clear(); TGT.clear();
		}
		WSACleanup();
	}
};

#endif // !__KDCSERVER_H_