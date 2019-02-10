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
#define INITCODE code_seg("INIT")
#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")


#define IOCTL_READ_PORT     CTL_CODE( \
                            FILE_DEVICE_UNKNOWN, \
                            0x800, \
                            METHOD_BUFFERED, \
                            FILE_ANY_ACCESS)


#define IOCTL_WRITE_PORT    CTL_CODE( \
                            FILE_DEVICE_UNKNOWN, \
                            0x800 + 1, \
                            METHOD_BUFFERED, \
                            FILE_ANY_ACCESS)




typedef struct _PORT_STRUCT
{
    USHORT  wPort;
    ULONG   dwPortVal;
    UCHAR   ucSize;
} PORT_STRUCT, PPORT_STRUCT;




typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT pDevice;
    UNICODE_STRING ustrDeviceName;
    UNICODE_STRING ustrSymLinkName;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;



NTSTATUS CreateDevice(
    IN	PDRIVER_OBJECT pDriverObject
);

VOID IODrvUnload(
    IN	PDRIVER_OBJECT pDriverObject
);


NTSTATUS IODrvDefaultDispatch(
    IN PDEVICE_OBJECT   pDevObj,
    IN PIRP             pIRP
);


NTSTATUS IODrvIOControl(
    IN PDEVICE_OBJECT   pDevObj,
    IN PIRP             pIRP
);