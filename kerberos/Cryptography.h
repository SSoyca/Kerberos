#ifndef CRYPTOGRAPHY_H_
#define CRYPTOGRAPHY_H_

/**********************************
	密码学相关的函数
Functions:
	

***********************************/

#include<iostream>
#include<string>
#include<ctime>
#include"General.h"
#include"md5.h"
//#include<wincrypt.h>

using namespace std;

/*
Function:
	加密函数
Parameters:
	key: 密钥
	type: 加密类型
Return:
	string： 以string类型返回加密后的字符串
*/


const long int getTime();
const string encrypt(const string key, const string msg, const int type);
const string decrypt(const string key, const string msg, const int type);
const string genHash(const string msg, const int type);
const string genSessionKey(const int type);
const string getPasswdof(const string name);
string GenMD5(unsigned char encrypt[]);

/*返回int类型时间*/
const long int getTime()
{
	time_t t;
	time(&t);
	return (long int)t;
}

const string encrypt(const string key,const string msg, const int type = 1)
{
	//根据type使用key进行不同类型的加密
	string res = "";
	for (int i = 0; i < msg.size(); i++)
	{
		if (msg[i] == '1')
		{
			res += '*'; 
		}
		else if (msg[i] == '+')
		{ 
			res += '!';
		}
		else
		{
			res += msg[i];
		}
	}

	return res;
}

const string decrypt(const string key, const string msg, const int type = 1)
{
	//解密算法
	string res = "";
	for (int i = 0; i < msg.size(); i++)
	{
		if (msg[i] == '*')
		{
			res += '1';
		}
		else if (msg[i] == '!')
		{
			res += '+';
		}
		else
		{
			res += msg[i];
		}
	}

	return res;
}

const string genHash(const string msg, const int type = 1)
{
	unsigned char* tmp = (unsigned char*)msg.c_str();
	return GenMD5(tmp); //计算哈希值
}

const string genSessionKey(const int type = 1)
{
	string res = to_string(getTime());
	return res;
}

//获取指定的用户的密钥
const string getPasswdof(const string name)
{
	return name;
}

string GenMD5(unsigned char encrypt[])
{
	string res = "";
	int i;
	unsigned char decrypt[16];

	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5, encrypt, strlen((char*)encrypt));
	MD5Final(&md5, decrypt);

	for (i = 0; i < 16; i++)
	{
		res += decrypt[i];
	}
	return res;
}

#endif // CRYPTOGRAPHY_H_