#include<ntddk.h>
#include<ntimage.h>

/*ϵͳ�����SSDT*/
typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG	ServiceTableBase;//������ַ���׵�ַ
	PULONG	ServiceCounterTableBase;//��������ÿ�����������ô���
	ULONG	NumberOfService;//����������
	ULONG	ParamTableBse;//�����������׵�ַ

}KSYSTEM_SERVICE_TABLE,*PKSYSTEM_SERVICE_TABLE;

/*���������� SSDT\ShadowSSDT\������\������*/
typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
	KSYSTEM_SERVICE_TABLE	ntoskrnl;//ntoskrnl.exe�ķ���������ָ��SSDT
	KSYSTEM_SERVICE_TABLE	win32k;//win32k.sys�ķ�����,��ShadowSSDT
	KSYSTEM_SERVICE_TABLE	UnUsed1;//��δʹ��
	KSYSTEM_SERVICE_TABLE	UnUsed2;//��δʹ��
}KSERVICE_TABLE_DESCRIPTOR,*PKSERVICE_TABLE_DESCRIPTOR;
/*SSDT�������*/
typedef struct _SSDT_ORDINAL
{
	unsigned	Ordinal:	12;//������
	unsigned	Table:	1;//������ 0=SSDT 1=ShadowSSDT
	unsigned	Unused:	19;//��δʹ��
}SSDT_ORDINAL,*PSSDT_ORDINAL;
/*CR0�Ĵ���*/
typedef struct _CR0
{
	unsigned	pe:		1;//PE����ģʽ��־λ
	unsigned	mp:		1;//MP���Э����λ
	unsigned	em:		1;//EMģ��Э����λ
	unsigned	ts:		1;//TS����ת��
	unsigned	et:		1;//ET��չ����λ
	unsigned	ne:		1;//NE ���ִ���
	unsigned	unUserd1:	10;// ��δʹ��
	unsigned	wp:		1;//WPд����λ 
	unsigned	unUserd2:	1;//��δʹ��
	unsigned	am:		1;//AM ������
	unsigned	unUserd3:	10;//��δʹ��
	unsigned	nw:		1;//NW not Writethrough
	unsigned	cd:		1;//CD �ɻ����־
	unsigned	pg:		1;//PG ��ҳ��־
}CR0,*PCR0;
/*Ҫhook�ĺ�����Ҫ�õ��Ľṹ��*/
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;//winternl.h,ͬʱ��������
//����ԭʼAPI��Ϣ
typedef NTSTATUS (* NTQUERYSYSTEMINFORMATION)(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);
/*ʵ���Լ��ĺ���*/
NTSTATUS NTAPI MyZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);
/*���������Ϣ�Ľṹ��*/
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    unsigned char  Reserved1[52];
    PVOID Reserved2[3];
    HANDLE UniqueProcessId;
    PVOID Reserved3;
    ULONG HandleCount;
    unsigned char  Reserved4[4];
    PVOID Reserved5[11];
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

PKSYSTEM_SERVICE_TABLE FindSSDTTable();//����SSDT��
VOID   KillProtect();//�ر�ֻ������
VOID   StartProtect();//����ֻ������
VOID UnLoadDriver(PDRIVER_OBJECT pDriverObject);//ж������
