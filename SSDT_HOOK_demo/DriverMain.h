#include<ntddk.h>
#include<ntimage.h>

/*系统服务表SSDT*/
typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG	ServiceTableBase;//函数地址表首地址
	PULONG	ServiceCounterTableBase;//函数表中每个函数被调用次数
	ULONG	NumberOfService;//服务函数个数
	ULONG	ParamTableBse;//参数个数表首地址

}KSYSTEM_SERVICE_TABLE,*PKSYSTEM_SERVICE_TABLE;

/*服务描述表 SSDT\ShadowSSDT\保留项\保留项*/
typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
	KSYSTEM_SERVICE_TABLE	ntoskrnl;//ntoskrnl.exe的服务函数，即指向SSDT
	KSYSTEM_SERVICE_TABLE	win32k;//win32k.sys的服务函数,即ShadowSSDT
	KSYSTEM_SERVICE_TABLE	UnUsed1;//暂未使用
	KSYSTEM_SERVICE_TABLE	UnUsed2;//暂未使用
}KSERVICE_TABLE_DESCRIPTOR,*PKSERVICE_TABLE_DESCRIPTOR;
/*SSDT调用序号*/
typedef struct _SSDT_ORDINAL
{
	unsigned	Ordinal:	12;//索引号
	unsigned	Table:	1;//服务表号 0=SSDT 1=ShadowSSDT
	unsigned	Unused:	19;//并未使用
}SSDT_ORDINAL,*PSSDT_ORDINAL;
/*CR0寄存器*/
typedef struct _CR0
{
	unsigned	pe:		1;//PE保护模式标志位
	unsigned	mp:		1;//MP监控协处理位
	unsigned	em:		1;//EM模拟协处理位
	unsigned	ts:		1;//TS任务转换
	unsigned	et:		1;//ET扩展类型位
	unsigned	ne:		1;//NE 数字错误
	unsigned	unUserd1:	10;// 暂未使用
	unsigned	wp:		1;//WP写保护位 
	unsigned	unUserd2:	1;//暂未使用
	unsigned	am:		1;//AM 对其标记
	unsigned	unUserd3:	10;//暂未使用
	unsigned	nw:		1;//NW not Writethrough
	unsigned	cd:		1;//CD 可缓存标志
	unsigned	pg:		1;//PG 分页标志
}CR0,*PCR0;
/*要hook的函数需要用到的结构体*/
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
} SYSTEM_INFORMATION_CLASS;//winternl.h,同时包含报错
//保存原始API信息
typedef NTSTATUS (* NTQUERYSYSTEMINFORMATION)(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);
/*实现自己的函数*/
NTSTATUS NTAPI MyZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);
/*保存进程信息的结构体*/
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

PKSYSTEM_SERVICE_TABLE FindSSDTTable();//查找SSDT表
VOID   KillProtect();//关闭只读保护
VOID   StartProtect();//开启只读保护
VOID UnLoadDriver(PDRIVER_OBJECT pDriverObject);//卸载驱动
