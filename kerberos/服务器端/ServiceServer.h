#ifndef SERVICESERVER_H_
#define SERVICESERVER_H_

/***********************************************
	��Windows�����е�Ӧ�÷������˷��ʿ���ͷ�ļ�
	���ܣ�
		1 ʵ����ͻ���֮���Kerberos���ʿ�������
		2 Э�̺�������ĻỰ��Կ
		3 ���������쳣��Ϣ
	todo��
		1 �����Ż���ע������
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
	int time_now;						//����ʱ��
	int time_delay = 30;				//����ӳ�ʱ��
	size_t Error;						//���շ���ֵ
	int SSrvPort = 1505;				//SS�˿�
	int Service_Port = 1403;			//�ṩ����Ķ˿�
	int QueueLength = 3;				//�ͻ��˶��г���
	SOCKET ClientSocket;				//�ͻ���socket
	SOCKET SSrvSocket;					//��������socket
	char SendBuffer[BufferSize];		//���ͻ�����
	char RecvBuffer[BufferSize];		//���ջ�����
	string CtoSrvSessionKey;			//��������ͻ��˵ĻỰ��Կ
	WORD wsaVersion = MAKEWORD(2, 2);	//WSA�汾��
	time_t Time;


public:
	ServiceServer()
	{}

	//��������ʼ��
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

		Error = ::bind(SSrvSocket, (sockaddr *)&SSrv_Info, SSrv_info_length);  //ͨ��::bind()�ķ�ʽ���Բ�����std���bind����
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

		Error = ::bind(SSrvSocket, (sockaddr*)&SSrv_Info, SSrv_info_length);  //ͨ��::bind()�ķ�ʽ���Բ�����std���bind����
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
		SOCKET ClientSocket;				//�ͻ���socket
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

			// 1 ���տͻ��˷��͵�Client_to_Server Ticket������
			string CtoSrvTicket_temp;
			SocketRecv(ClientSocket, RecvBuffer, BufferSize);
			if (Error < 0)
			{
				cout << "ERROR!" << WSAGetLastError() << endl;
				exit(-1);
			}
			msgTemp = RecvBuffer;
			memset(RecvBuffer, 0, sizeof(RecvBuffer));
			/*ʹ���Լ�����Կ����*/
			CtoSrvTicket_temp = decrypt(getPasswdof("SS"), msgTemp);

			// 2 ���Client_to_Server Ticket
			vector<string> CtoSrvTicket;
			StringSplit(CtoSrvTicket, CtoSrvTicket_temp);
			if (CtoSrvTicket.size() != 4)
			{
				cout << "ERROR!" << endl;
				exit(-1);
			}
			/*���ʱЧ*/
			time_now = int(getTime());
			if (stoi(CtoSrvTicket[2], nullptr, 0) < time_now)
			{
				cout << "Timeout!" << endl;
				exit(-1);
			}
			CtoSrvSessionKey = CtoSrvTicket[0];
			ClientID = CtoSrvTicket[1];
			ClientMAC = CtoSrvTicket[3];

			// 3 ������֤��{ClientID,TimeStamp}
			SocketRecv(ClientSocket, RecvBuffer, BufferSize);
			msgTemp = RecvBuffer;
			memset(RecvBuffer, 0, sizeof(RecvBuffer));

			// 4 ������֤��
			string Auth_temp;
			Auth_temp = decrypt(CtoSrvSessionKey, msgTemp);
			vector<string> Authenticator;
			StringSplit(Authenticator, Auth_temp);
			if (Authenticator.size() != 2)
			{
				cout << "Error!" << endl;
				exit(-1);
			}

			// 4.5 ������֤
			if (strcmp(Authenticator[0].c_str(), ClientID.c_str()))
			{	//id��ͬ
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

			// 5 ����ʱ���
			TimeStamp = encrypt(CtoSrvSessionKey, Authenticator[1]);
			StringToChar(SendBuffer, TimeStamp);
			SocketSend(ClientSocket, SendBuffer);

			// 6 ��֤ͨ��
			closesocket(ClientSocket);
		}
		MessageExchange();
		WSACleanup();
	}

};


#endif // !SERVICESERVER_H_