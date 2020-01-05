#ifndef GENERAL_H_
#define GENERAL_H_

/* 
 
   通用头文件，提供一些简单的基本操作实现
 
*/
#pragma comment(lib,"WS2_32.lib")

#include<WinSock2.h>
#include<WS2tcpip.h>
#include<errno.h>
#include<string>
#include<vector>
#include<iostream>
#include<Windows.h>
#include<boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

const char* PLUS = "+";  //使用+号分隔 

void StringToChar(char Dest[], const string Source);
void SocketSend(SOCKET s, const char* Buffer, int flag);
void SocketRecv(SOCKET s, char* Buffer, const int BufferSize, int flag);
void SocketErrorCheck(int ErrorCode);
void SegmentCleanUp(char* buf);
void StringSplit(vector<string>& Dest, string& Source, const char Pred[]);
//void CharToString(string Dest, const char Source[]); //已弃用 

void StringToChar(char Dest[], const string Source)
{
    int err = 0;
    try
    {
        SegmentCleanUp(Dest);
        err = strcpy_s(Dest, Source.size() + 1, Source.c_str());
    }
    catch (...)
    {
        cerr << "(" << err << ") Error in Function StringToChar(): size of Source is " << Source.size() + 1
            << " while length of Destination is " << sizeof(Dest) << endl;
    }
}

void SocketErrorCheck(int ErrorCode)
{
    if (ErrorCode < 0)
    {
        cerr << "Socket Error Occurred! ErrorCode: " << WSAGetLastError() << endl;
    }
}

inline void SegmentCleanUp(char* buf)
{
    try
    {
        memset(&buf, 0, sizeof(buf));
    }
    catch (...)
    {
        cerr << "Unexpected Error at Function SegmemtCleanUp()" << endl;
    }
}

inline void StringSplit(vector<string>& Dest, string& Source, const char Pred[] = PLUS)
{
    split(Dest, Source, is_any_of(Pred), token_compress_off);
}

inline void SocketSend(SOCKET s, const char* Buffer, int flag = 0)
{
    Sleep(700);
    SocketErrorCheck(::send(s, Buffer, strlen(Buffer) + 1, flag));
}

inline void SocketRecv(SOCKET s, char* Buffer, const int BufferSize, int flag = 0)
{
    SocketErrorCheck(recv(s, Buffer, BufferSize, flag));
}

/* // 应对string 指针进行操作，得不偿失，弃用
void CharToString(string Dest, const char Source[])
{
    try
    {
        Dest.clear();
        Dest = Source;
    }
    catch (...)
    {
        cerr << "Unexpected Error Occurred at Function CharToString()" << endl;
    }

}
*/

#endif // !GENERAL_H_