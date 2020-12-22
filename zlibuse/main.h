#include <windows.h>
#include<stdio.h>
#include<shlwapi.h>
#include "zconf.h"
#include "zlib.h"

#pragma comment(lib,"zlibstat.lib")
#pragma comment(lib,"shlwapi.lib")


#define MAX_SRC_FILE_SIZE (500*1024*1024)//500M

void Zlib_ShowError(char *pszText)
{
	char szErr[MAX_PATH] = {0};
	sprintf(szErr,"%s Error[%d]\n",pszText,GetLastError());
	MessageBox(NULL,szErr,"ERROR",MB_OK | MB_ICONERROR);

}
// ����ѹ��
// ���룺��Ҫѹ���ļ���·��
// ���������ѹ�����ѹ���������ݡ�����ѹ�����ѹ���������ݳ���
BOOL Zlib_CompressData(char *pszCompressFileName,BYTE ** ppCompressData,DWORD *pdwCompressDataSize)
{
	//*************ѹ���ļ�********************//
	HANDLE hFile = CreateFile(pszCompressFileName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		Zlib_ShowError("CreateFile");
		return FALSE;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);       // ��ȡ�ļ���С
	if (MAX_SRC_FILE_SIZE < dwFileSize)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	// �ж��Ƿ������С��������
	if (MAX_SRC_FILE_SIZE < dwFileSize)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	DWORD dwDestDataSize = dwFileSize;

	BYTE *pSrcData = new BYTE[dwFileSize];
	if (NULL == pSrcData)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	BYTE *pDestData = new BYTE[dwDestDataSize];
	if (NULL == pDestData)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	DWORD dwRet = 0;
	ReadFile(hFile, pSrcData, dwFileSize, &dwRet, NULL);	 // ��ȡ�ļ�����
	if ((0 >= dwRet) ||
		(dwRet != dwFileSize))
	{
		delete[]pDestData;
		pDestData = NULL;
		delete[]pSrcData;
		pSrcData = NULL;
		CloseHandle(hFile);
		return FALSE;
	}

	// ѹ������
	/*
	int compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
	compress������source�������е�����ѹ����dest��������sourceLen��ʾsource�������Ĵ�С�����ֽڼƣ���
	ע�⣺�����ĵڶ�������destLen�Ǵ�ַ���ã������ú���ʱ��destLen��ʾdest��������С����ʼֵ����Ϊ0Ŷ����
	( destLen > (sourceLen + 12) * 100.1% )���������˳���destLen��ʾѹ���󻺳�����ʵ�ʴ�С��
	��ʱ��destLen/sourceLen������ѹ����!!!
	����ֵ��
	-5 : ���������������
	-4 : û���㹻���ڴ棻
	0 : ��ʾ�ɹ���
	*/
	int iRet = 0;
	do
	{
		iRet = compress(pDestData, &dwDestDataSize, pSrcData, dwFileSize);
		if (0 == iRet)
		{
			// �ɹ�
			break;
		}
		else if (-5 == iRet)
		{
			// ���������������, �� 100KB ��С����
			delete[]pDestData;
			pDestData = NULL;
			dwDestDataSize = dwDestDataSize + (100 * 1024);
			pDestData = new BYTE[dwDestDataSize];
			if (NULL == pDestData)
			{
				delete[]pSrcData;
				pSrcData = NULL;
				CloseHandle(hFile);
				return FALSE;
			}
		}
		else
		{
			// û���㹻���ڴ� �� �������
			delete[]pDestData;
			pDestData = NULL;
			delete[]pSrcData;
			pSrcData = NULL;
			CloseHandle(hFile);
			return FALSE;
		}
	} while (TRUE);
	// ��������
	*ppCompressData = pDestData;
	*pdwCompressDataSize = dwDestDataSize;

	// �ͷ�
	//	delete[]pDestData;
	//	pDestData = NULL;
	delete[]pSrcData;
	pSrcData = NULL;
	CloseHandle(hFile);

	return TRUE;
}
BOOL SaveToFile(char *pszFileName,BYTE *pData,DWORD dwDataSize)
{
	HANDLE hFile = CreateFile(pszFileName,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,NULL);
	DWORD dwRet = 0;
	WriteFile(hFile,pData,dwDataSize,&dwRet,NULL);
	CloseHandle(hFile);

	return TRUE;
}

// ���ݽ�ѹ
// ���룺��Ҫ��ѹ���ļ���·��
// ��������ݽ�ѹ����������ݡ����ݽ�ѹ������ݳ���
BOOL Zlib_UncompressData(char *pszUncompressFileName, BYTE **ppUncompressData, DWORD *pdwUncompressDataSize)
{
	// ע����ܳ���ѹ������ļ���ѹ��ǰ���ļ��������!!!

	// ���ļ� �� ��ȡ�ļ�����
	HANDLE hFile = CreateFile(pszUncompressFileName, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		Zlib_ShowError("CreateFile");
		return FALSE;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);       // ��ȡ�ļ���С
	DWORD dwDestDataSize = MAX_SRC_FILE_SIZE;

	BYTE *pSrcData = new BYTE[dwFileSize];
	if (NULL == pSrcData)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	BYTE *pDestData = new BYTE[dwDestDataSize];
	if (NULL == pDestData)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	DWORD dwRet = 0;
	ReadFile(hFile, pSrcData, dwFileSize, &dwRet, NULL);	 // ��ȡ�ļ�����
	if ((0 >= dwRet) ||(dwRet != dwFileSize))
	{
		delete[]pDestData;
		pDestData = NULL;
		delete[]pSrcData;
		pSrcData = NULL;
		CloseHandle(hFile);
		return FALSE;
	}

	// ��ѹ������
	/*
	int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
	compress������source�������е�����ѹ����dest��������sourceLen��ʾsource�������Ĵ�С�����ֽڼƣ���
	ע�⣺�����ĵڶ�������destLen�Ǵ�ַ���ã������ú���ʱ��destLen��ʾdest��������С����ʼֵ����Ϊ0Ŷ����
	( destLen > (sourceLen + 12) * 100.1% )���������˳���destLen��ʾѹ���󻺳�����ʵ�ʴ�С��
	��ʱ��destLen/sourceLen������ѹ����!!!
	����ֵ��
	-5 : ���������������
	-4 : û���㹻���ڴ棻
	0 : ��ʾ�ɹ���
	*/
	int iRet = 0;
	do
	{
		iRet = uncompress(pDestData, &dwDestDataSize, pSrcData, dwFileSize);
		if (0 == iRet)
		{
			// �ɹ�
			break;
		}
		else if (-5 == iRet)
		{
			// ���������������, �� 100KB ��С����
			delete[]pDestData;
			pDestData = NULL;
			dwDestDataSize = dwDestDataSize + (100 * 1024);
			pDestData = new BYTE[dwDestDataSize];
			if (NULL == pDestData)
			{
				delete[]pSrcData;
				pSrcData = NULL;
				CloseHandle(hFile);
				return FALSE;
			}
		}
		else
		{
			// û���㹻���ڴ� �� �������
			delete[]pDestData;
			pDestData = NULL;
			delete[]pSrcData;
			pSrcData = NULL;
			CloseHandle(hFile);
			return FALSE;
		}
	} while (TRUE);
	// ��������
	*ppUncompressData = pDestData;
	*pdwUncompressDataSize = dwDestDataSize;

	// �ͷ�
	//	delete[]pDestData;
	//	pDestData = NULL;
	delete[]pSrcData;
	pSrcData = NULL;
	CloseHandle(hFile);

	return TRUE;
}