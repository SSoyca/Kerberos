#ifndef CRYPTOGRAPHY_H_
#define CRYPTOGRAPHY_H_

/**********************************
	����ѧ��صĺ���
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
	���ܺ���
Parameters:
	key: ��Կ
	type: ��������
Return:
	string�� ��string���ͷ��ؼ��ܺ���ַ���
*/


const long int getTime();
const string encrypt(const string key, const string msg, const int type);
const string decrypt(const string key, const string msg, const int type);
const string genHash(const string msg, const int type);
const string genSessionKey(const int type);
const string getPasswdof(const string name);
string GenMD5(unsigned char encrypt[]);

/*����int����ʱ��*/
const long int getTime()
{
	time_t t;
	time(&t);
	return (long int)t;
}

const string encrypt(const string key,const string msg, const int type = 1)
{
	//����typeʹ��key���в�ͬ���͵ļ���
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
	//�����㷨
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
	return GenMD5(tmp); //�����ϣֵ
}

const string genSessionKey(const int type = 1)
{
	string res = to_string(getTime());
	return res;
}

//��ȡָ�����û�����Կ
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