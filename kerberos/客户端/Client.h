#ifndef CLIENT_H_
#define CLIENT_H_

/**********************************
	此版本客户端在Windows下运行
	功能：
		1 实现Kerberos流程 - done

	todo：
		自动探测AS服务器 

***********************************/

#pragma comment(lib,"WS2_32.lib")
#include"../Cryptography.h"
#define BufferSize 256	//缓冲区长度

using namespace std;

class Client
{
private:
	int kdcPort = 1500;					//KDC服务器使用的端口 
	int tgsPort = 1501;					//TGS服务器使用的端口 
	int ssrvPort = 1505;				//SS验证使用的端口 
	int ServicePort = 1403;				//SS服务端口
	int Error = 0;						//保存函数返回状态值 
	char SendBuffer[BufferSize];		//发送缓冲 
	char ReceiveBuffer[BufferSize];		//接收缓冲 
	WSADATA ClientWSA;					//WSA对象 
	WORD wVersion = MAKEWORD(2, 2);		//WSA版本 
	sockaddr_in AuthSrv_addr;			//服务器地址和端口 
	SOCKET ClientSockToKDC;				//用于连接KDC的socket对象 
	SOCKET ClientSockToTGS;				//用于连接TGS的socket对象 
	SOCKET ClientSockToSS;				//用于连接SS的socket对象 
	string ClientID, TimeStamp;			//保存用户ID，时间戳信息 
	string CSSessionKey;				//客户端与服务器的会话密钥
	HANDLE hStdin;						//stdin流 
	DWORD mode;							//mode 
	time_t Time;						//时间 

	struct userInfo
	{
		string userName;
		string passWord;
		userInfo(int)
		{
			userName.clear();
			passWord.clear();
		}
		userInfo(string name, string pswd)
		{
			userName = name;
			passWord = pswd;
		}
		userInfo() 
		{
			userName.clear();
			passWord.clear();
		}
		bool empty()
		{
			if (userName.empty() || passWord.empty())
				return true;
			else
				return false;
		}
	};

public:
	Client()
	{
		hStdin = GetStdHandle(STD_INPUT_HANDLE);
		mode = 0;
	}

	/*
	Function:
		客户端启动函数，用于设定基本的网络参数和实例化对象
	Parameters:
		void: 无
	Return:
		bool: 成功完成返回true，否则返回false
	*/
	bool startup()
	{
		Error = WSAStartup(wVersion, &ClientWSA);
		if (Error != 0)
		{
			cout << "error!" << WSAGetLastError() << endl;	//报错退出
			exit(-1);
		}

		ClientSockToKDC = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ClientSockToKDC == SOCKET_ERROR)
		{
			cout << "socketError!" << WSAGetLastError() << endl;//socket启动错误退出
			exit(-1);
		}

		return true;
	}

	//自动搜索当前网络中的鉴权服务器（kdc和tgs）
	bool ASDetect()
	{
		;
	}

	/* 
	Function:
		设定服务器地址和端口
	Parameters:
		srv_addr: sockaddr_in类型，用于直接替换AuthSrv_addr的值
	Return:
		void： 无返回值 
	*/
	void SetAuthSrvAddr(sockaddr_in srv_addr)
	{
		this->AuthSrv_addr = srv_addr;
	}

	/*
	Function:
		设定服务器地址和端口，通过内置的cin完成IP地址的设定，端口直接使用ServicePort
	Parameters:
		void: 无
	Return:
		sockaddr_in: 返回一个socket地址类型
	*/
	sockaddr_in SetAddressANDPort()
	{
		string input_tmp;
		sockaddr_in srvadr;
		cout << "ip:";  //输入检测！！
		cin >> input_tmp;
		const char* input_addr = input_tmp.c_str();
		inet_pton(AF_INET, input_addr, &srvadr.sin_addr.S_un.S_addr);
		srvadr.sin_port = ServicePort;
		return srvadr;
	}

	/*
	Function:
		用于用户登录的函数，仅当没有设定用户信息时使用
	Parameters:
		User: UserInfo结构体类型，如果用户信息为空则使用cin进行设定
	Return:
		userInfo: 返回设定好的用户信息结构体 
	*/
	userInfo login(userInfo User = NULL)
	{
		if (User.empty())
		{
			cout << "Login:" << endl;
			cout << "Username: ";
			cin >> User.userName;
			cout << "Password: ";
			EchoSwitch(false);  //关闭回显隐藏输入
			cin >> User.passWord;
			EchoSwitch(true);	//使用后重新开启
			User.passWord = genHash(User.passWord);
		}
		return User;
	}

	/*
	Function:	
		开关console输入回显，用于隐藏密码输入，仅在控制台时有效
	Parameters:
		enableEcho: Boolean类型，值为true则开启输入回显
	Return:
		void： 无返回值
	*/
	void EchoSwitch(bool enableEcho = true)
	{
		if (!enableEcho) 
		{
			GetConsoleMode(hStdin, &mode);
			SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
		}
		else
		{
			SetConsoleMode(hStdin, mode);
		}
	}

	/*
	Function:
		简单的消息交换，使用已经协商完成的会话密钥加密并交换的第一条消息
	Parameters:
		void: 无
	Return:
		void: 无
	*/
	void MessageExchange()
	{
		string msg;
		sockaddr_in srvAddr = SetAddressANDPort();
		SOCKET cTosSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (cTosSocket == SOCKET_ERROR)
		{
			cerr << "socket init failed!" << endl;
			exit(-1);
		}
		Error = connect(cTosSocket, (SOCKADDR*)&srvAddr, sizeof(ServicePort));
		if (Error == SOCKET_ERROR)
		{
			WSACleanup();
			exit(-1);
		}

		msg = "helloWorld!";
		StringToChar(SendBuffer, encrypt(CSSessionKey, msg));
		SocketSend(cTosSocket, SendBuffer);
		closesocket(cTosSocket);		
	}

	/*
	Function:
		主运行函数，将个部分函数按流程运行以完成完整的访问控制流程
	Parameters:
		void： 无
	Return:
		bool：成功完成则返回true，否则返回false
	*/
	bool Run()
	{
		string msgTmp;
		string input_tmp;
		cout << "ip:";  //输入检测！！
		cin >> input_tmp;
		const char* input_addr = input_tmp.c_str();

		//提取函数! 
		sockaddr_in ASrvaddr;
		ASrvaddr.sin_family = AF_INET;
		ASrvaddr.sin_port = htons(kdcPort);
		inet_pton(AF_INET, input_addr, &ASrvaddr.sin_addr.S_un.S_addr);

		SetAuthSrvAddr(ASrvaddr);
		startup();
		Error = connect(ClientSockToKDC, (SOCKADDR*)&AuthSrv_addr, sizeof(AuthSrv_addr));
		if (Error == SOCKET_ERROR)
		{
			cout << "ERROR!" << WSAGetLastError() << endl;//connect报错退出
			WSACleanup();
			exit(-1);
		}

		/* 开始认证流程 */

		// 1 发送用户名给kdc 
		userInfo userTest("zcy", "abcdef"); //debug!
		
		const userInfo user = login(userTest);  //用户输入用户名和密码，返回的密码为输入密码的哈希值 
		msgTmp = user.userName;  
		if (msgTmp.size() > BufferSize)
		{
			cout << "ERROR!";  //数组越界退出
			return false;
		}
		StringToChar(SendBuffer, msgTmp);
		SocketSend(ClientSockToKDC, SendBuffer);

		// 2 接收TGS会话密钥和TGT 
		string ClientTGSSessionKey, TGT;
		SocketRecv(ClientSockToKDC, ReceiveBuffer, BufferSize);
		msgTmp = ReceiveBuffer;
		ClientTGSSessionKey = decrypt(user.passWord, msgTmp);
		SocketRecv(ClientSockToKDC, ReceiveBuffer, BufferSize);
		msgTmp = ReceiveBuffer;
		TGT = msgTmp;
		
		// 3 验证TGS正确性 
		vector<string> TGS_CHECK;
		StringSplit(TGS_CHECK, ClientTGSSessionKey);	//按加号分割接收到的tgs明文 
		if (TGS_CHECK[0] != user.userName)
		{
			cout << "Error Occurred at Checking TGS!" << endl;	//收到的tgs校验失败
			return false;
		}
		ClientID = TGS_CHECK[2];
		TGS_CHECK.clear();
		closesocket(ClientSockToKDC); //断开与kdc的连接 

		// 3.5 选择要访问的服务器并获得对应的服务ID ！没写完！ 
		string ServiceID = "media-2222";

		// 4 连接到TGS服务器 
		ClientSockToTGS = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ClientSockToTGS == SOCKET_ERROR)
		{
			cout << "socketError!" << WSAGetLastError() << endl;//socket启动错误退出
			exit(-1);
		}
		//重新设定socket并连接到TGS服务器 
		ASrvaddr.sin_port = htons(tgsPort);
		SetAuthSrvAddr(ASrvaddr);
		Error = connect(ClientSockToTGS, (SOCKADDR*)&AuthSrv_addr, sizeof(AuthSrv_addr));
		if (Error == SOCKET_ERROR)
		{
			cout << "ERROR!" << WSAGetLastError() << endl;//connect报错退出
			WSACleanup();
			exit(-1);
		}

		// 5 发送要请求的服务器ID+TGT和验证器{用户ID，时间戳}
		msgTmp = ServiceID + "+" + TGT;
		StringToChar(SendBuffer, msgTmp);
		SocketSend(ClientSockToTGS, SendBuffer);
		msgTmp.clear();
		SegmentCleanUp(SendBuffer);
		string Authenticator = ClientID + "+" + to_string(getTime());
		StringToChar(SendBuffer, Authenticator);
		SocketSend(ClientSockToTGS, SendBuffer);
		SegmentCleanUp(SendBuffer);


		// 6 接收TGS发送的CTST(Client-To-Server Ticket)和C-SSK(Client-Server SessionKey) 
		string CtoSTicket;
		SocketRecv(ClientSockToTGS, ReceiveBuffer, BufferSize);
		msgTmp = ReceiveBuffer;
		CtoSTicket = msgTmp;
		msgTmp.clear();
		memset(ReceiveBuffer, 0, BufferSize);
		SocketRecv(ClientSockToTGS, ReceiveBuffer, BufferSize);
		msgTmp = ReceiveBuffer;
		CSSessionKey = decrypt(ClientTGSSessionKey, msgTmp); 
		msgTmp.clear();
		memset(ReceiveBuffer, 0, BufferSize);

		// 7 建立与目标服务器的连接 
		closesocket(ClientSockToTGS);
		ClientSockToSS = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ClientSockToSS == SOCKET_ERROR)
		{
			exit(-1);
		}
		ASrvaddr.sin_port = htons(ssrvPort);

		SetAuthSrvAddr(ASrvaddr);
		Error = connect(ClientSockToSS, (SOCKADDR*)&AuthSrv_addr, sizeof(AuthSrv_addr));
		if (Error == SOCKET_ERROR)
		{
			WSACleanup();
			exit(-1);
		}

		// 8 向服务器发送服务请求消息：CtoSTicket和验证器{ClientID,TimeStamp} 
		string Authenticator2;
		StringToChar(SendBuffer, CtoSTicket);
		SocketSend(ClientSockToSS, SendBuffer);
		TimeStamp = to_string(getTime());
		Authenticator2 = encrypt(CSSessionKey, ClientID + '+' + TimeStamp);
		StringToChar(SendBuffer, Authenticator2);
		SocketSend(ClientSockToSS, SendBuffer);
		Authenticator2.clear();
		memset(SendBuffer, 0, BufferSize);

		// 9 接收并验证SS发送的消息 
		SocketRecv(ClientSockToSS, ReceiveBuffer, BufferSize);
		msgTmp = ReceiveBuffer;
		msgTmp = decrypt(CSSessionKey, msgTmp);
		if (msgTmp != TimeStamp)
		{
			cout << "Error!" << endl;
			exit(-1);
		}
		closesocket(ClientSockToSS);

		//尝试使用约定的参数进行消息传送
		MessageExchange();

		WSACleanup();

		return true;
	}

};

#endif // !__CLIENT_H_ 