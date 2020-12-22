#include "main.h"

int main(int argc,char *argv[],char *envp[])
{
	BOOL bRet = FALSE;
	BYTE *pCompressData = NULL;
	DWORD dwCompressDataSize = 0;

	BYTE *pUncompressData = NULL;
	DWORD dwUncompressDataSize = 0;
	
	bRet = Zlib_CompressData(argv[1],&pCompressData,&dwCompressDataSize);
	//BOOL Zlib_CompressData(char *pszCompressFileName,BYTE ** ppCompressData,DWORD *pdwCompressDataSize)
	if(bRet == FALSE)
	{
		Zlib_ShowError("Zlib_CompressData fail!");
		delete [] pCompressData;
		pCompressData = NULL;
		return 0;
	}

	SaveToFile("yeanhoo.myzip",pCompressData,dwCompressDataSize);
	bRet = Zlib_UncompressData("yeanhoo.myzip",&pUncompressData,&dwUncompressDataSize);
	//BOOL Zlib_UncompressData(char *pszUncompressFileName, BYTE **ppUncompressData, DWORD *pdwUncompressDataSize)
	if(bRet == FALSE)
	{
		Zlib_ShowError("Zlib_UncompressData fail!");
		delete [] pUncompressData;
		pUncompressData = NULL;
		delete [] pCompressData;
		pCompressData = NULL;
		return 0;
	}
	SaveToFile("yeanhoo.exe",pUncompressData,dwUncompressDataSize);
	delete [] pUncompressData;
	pUncompressData = NULL;
	delete [] pCompressData;
	pCompressData = NULL;

	getchar();
	return 0;
}

