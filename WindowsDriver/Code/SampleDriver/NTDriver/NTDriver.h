#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntddk.h>
#ifdef __cplusplus
}
#endif

#define PAGECODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT");
#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")


#define arraysize(p) (sizeof(p)/sizeof(p[0]))


typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


NTSTATUS CreateDevice(
	IN	PDRIVER_OBJECT pDriverObject
);

VOID HelloNTUnload(
	IN	PDRIVER_OBJECT pDriverObject
);


NTSTATUS HelloNTDispatchRoutine(
	IN	PDEVICE_OBJECT	pDevObj,
	IN	PIRP			pIRP
);

