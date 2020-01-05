#include<iostream>
#include<string>
#include"KDCserver.h"
#include"md5.h"

using namespace std;

int debug_main()
{
	KDCsrv AS_kdc;	
	AS_kdc.Run();

	return 0;
}

int dbo_main()
{
	//dbOperate dbo;
	//dbo.dbSearchUserName("zy");
	return 0;
}

int main() 
{
	//return dbo_main();
	return debug_main();
}

