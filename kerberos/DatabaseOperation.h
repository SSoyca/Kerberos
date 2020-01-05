#ifndef DATABASEOPERATION_H_
#define DATABASEOPERATION_H_

/*
 
      此头文件中提供简单操作数据库的方法：
        查找指定表项
        插入项

*/

#define CMDLength 128
#include<icrsint.h>
#include<Windows.h>
#include<iostream>
#include<cstring>
#include<string>
#include<stdlib.h>
#include"General.h"
#import "C:/Program Files/Common Files/System/ado/msado15.dll" no_namespace rename("EOF","EndOfFile")
using namespace std;
inline void TESTHR(HRESULT x) { if FAILED(x) _com_issue_error(x); };
/*
class dbOperate
{
private:
	_ConnectionPtr dbConnection;
	_CommandPtr dbCmd;
	_RecordsetPtr dbRecord;
	_variant_t RecordAffected;
	_variant_t vUsername, vID, vPassword;
	HRESULT hr = S_OK;
	IADORecordBinding* picRs = NULL;

public:
	dbOperate(const string Catalog = "KDC_MAIN", const string LoginID = "kerberos", const string Passwd = "kerberos")
	{
		char ConnectCMD[CMDLength];
		string temp = "Provider=SQLOLEDB;Data Source=localhost;Initial Catalog=";
		temp += Catalog + ";User ID="; temp += LoginID + ";Password="; temp += Passwd;
		StringToChar(ConnectCMD, temp);
		_bstr_t ConnectStr = ConnectCMD;
		try
		{
			if (FAILED(CoInitialize(NULL)))
			{
				cerr << "ADO Start Failed!" << endl;
			}
			//创建连接实例
			auto dbHr = dbConnection.CreateInstance("ADODB.Connection");
			if (FAILED(dbHr))
			{
				cout << "Database Initialize Failed!" << endl;
			}
			dbHr = dbConnection->Open(ConnectStr, "", "", adModeUnknown);
		}
		catch (_com_error e)//错误捕捉
		{
			cout << e.Description() << endl;
		}
		cout << "a" << endl;
	}

	string dbSearchUserName(const char keyword[])
	{
		dbRecord.CreateInstance(__uuidof(Recordset));
		string tmp = keyword;
		string comd = "select * from KDC_User_table where UserName='zcy'";
		try
		{
			dbRecord->Open("select * from KDC_User_table where UserName='zcy'",
				dbConnection.GetInterfacePtr(),
				adOpenForwardOnly,
				adLockUnspecified, 
				adCmdText);
		}
		catch (_com_error * e)
		{
			cout << "Error Occurred!" << endl;
			exit(-1);
		}
		while (!dbRecord->adoEOF)
		{
			//vID = dbRecord->GetFields()->GetItem((long)0).Value;  
			//_variant_t strid = vID; 
			//vPassword = dbRecord->GetFields()->GetItem((long)0).name; 
			///vUsername = dbRecord->GetCollect("username"); 
			//dbRecord->MoveNext(); 
		}

		tmp.clear();
		return tmp;
	}

	bool dbClose()
	{
		dbRecord->Close();
		dbConnection->Close();
		CoUninitialize();
	}
	

	
};
*/

// BeginOpenCpp 

#include <oledb.h> 
#include <stdio.h> 
#include <conio.h> 
// Function declarations 
inline void TESTHR(HRESULT x) { if FAILED(x) _com_issue_error(x); };
void OpenX(void);
void PrintProviderError(_ConnectionPtr pConnection);
void PrintComError(_com_error& e);

/////////////////////////////////////////////////////////// 
// // 
// Main Function // 
// // 
/////////////////////////////////////////////////////////// 

void main()
{
    if (FAILED(::CoInitialize(NULL)))
        return;

    OpenX();

    ::CoUninitialize();
}

/////////////////////////////////////////////////////////// 
// // 
// OpenX Function // 
// // 
/////////////////////////////////////////////////////////// 

void OpenX(void)
{
    // Define ADO object pointers. 
    // Initialize pointers on define. 
    // These are in the ADODB:: namespace 
    _RecordsetPtr pRstEmployee = NULL;
    _ConnectionPtr pConnection = NULL;

    // Define string variables. 
    _bstr_t strCnn("Provider='sqloledb';Data Source='MySqlServer';"
        "Initial Catalog='pubs';Integrated Security='SSPI';");

    // Define Other Variables. 
    HRESULT hr = S_OK;
    IADORecordBinding* picRs = NULL; // Interface Pointer declared. 
    CEmployeeRs emprs; // C++ Class object 
    DBDATE varDate;

    try
    {
        // open connection and record set 
        TESTHR(pConnection.CreateInstance(__uuidof(Connection)));
        pConnection->Open(strCnn, "", "", adConnectUnspecified);

        TESTHR(pRstEmployee.CreateInstance(__uuidof(Recordset)));
        pRstEmployee->Open("Employee",
            _variant_t((IDispatch*)pConnection, true), adOpenKeyset,
            adLockOptimistic, adCmdTable);

        // Open an IADORecordBinding interface pointer which we'll 
        // use for Binding Recordset to a class. 
        TESTHR(pRstEmployee->QueryInterface(
            __uuidof(IADORecordBinding), (LPVOID*)&picRs));

        //Bind the Recordset to a C++ Class here. 
        TESTHR(picRs->BindToRecordset(&emprs));

        // Assign the first employee record's hire date 
        // to a variable, then change the hire date. 
        varDate = emprs.m_sze_hiredate;
        printf("\nOriginal data\n");
        printf("\tName - Hire Date\n");
        printf(" %s %s - %d/%d/%d\n\n",
            emprs.le_fnameStatus == adFldOK ?
            emprs.m_sze_fname : "<NULL>",
            emprs.le_lnameStatus == adFldOK ?
            emprs.m_sze_lname : "<NULL>",
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.month : 0,
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.day : 0,
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.year : 0);

        emprs.m_sze_hiredate.year = 1900;
        emprs.m_sze_hiredate.month = 1;
        emprs.m_sze_hiredate.day = 1;
        picRs->Update(&emprs);

        printf("\nChanged data\n");
        printf("\tName - Hire Date\n");
        printf(" %s %s - %d/%d/%d\n\n",
            emprs.le_fnameStatus == adFldOK ?
            emprs.m_sze_fname : "<NULL>",
            emprs.le_lnameStatus == adFldOK ?
            emprs.m_sze_lname : "<NULL>",
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.month : 0,
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.day : 0,
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.year : 0);

        // Requery Recordset and reset the hire date. 
        pRstEmployee->Requery(adOptionUnspecified);
        // Open an IADORecordBinding interface pointer which we'll 
        // use for Binding Recordset to a class. 
        TESTHR(pRstEmployee->QueryInterface(
            __uuidof(IADORecordBinding), (LPVOID*)&picRs));

        // Rebind the Recordset to a C++ Class here. 
        TESTHR(picRs->BindToRecordset(&emprs));
        emprs.m_sze_hiredate = varDate;
        picRs->Update(&emprs);
        printf("\nData after reset\n");
        printf("\tName - Hire Date\n");
        printf(" %s %s - %d/%d/%d", emprs.le_fnameStatus == adFldOK ?
            emprs.m_sze_fname : "<NULL>",
            emprs.le_lnameStatus == adFldOK ?
            emprs.m_sze_lname : "<NULL>",
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.month : 0,
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.day : 0,
            emprs.le_hiredateStatus == adFldOK ?
            emprs.m_sze_hiredate.year : 0);
    }
    catch (_com_error & e)
    {
        // Notify the user of errors if any. 
        // Pass a connection pointer accessed from the Connection. 
        PrintProviderError(pConnection);
        PrintComError(e);
    }

    // Clean up objects before exit. 
    if (pRstEmployee)
        if (pRstEmployee->State == adStateOpen)
            pRstEmployee->Close();
    if (pConnection)
        if (pConnection->State == adStateOpen)
            pConnection->Close();
}

/////////////////////////////////////////////////////////// 
// // 
// PrintProviderError Function // 
// // 
/////////////////////////////////////////////////////////// 

void PrintProviderError(_ConnectionPtr pConnection)
{
    // Print Provider Errors from Connection object. 
    // pErr is a record object in the Connection's Error collection. 
    ErrorPtr pErr = NULL;

    if ((pConnection->Errors->Count) > 0)
    {
        long nCount = pConnection->Errors->Count;
        // Collection ranges from 0 to nCount -1. 
        for (long i = 0; i < nCount; i++)
        {
            pErr = pConnection->Errors->GetItem(i);
            printf("\t Error number: %x\t%s", pErr->Number,
                pErr->Description);
        }
    }
}

/////////////////////////////////////////////////////////// 
// // 
// PrintComError Function // 
// // 
/////////////////////////////////////////////////////////// 

void PrintComError(_com_error& e)
{
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());

    // Print COM errors. 
    printf("Error\n");
    printf("\tCode = %08lx\n", e.Error());
    printf("\tCode meaning = %s\n", e.ErrorMessage());
    printf("\tSource = %s\n", (LPCSTR)bstrSource);
    printf("\tDescription = %s\n", (LPCSTR)bstrDescription);
}
// EndOpenCpp

#endif // !DATABASEOPERATION_H_
