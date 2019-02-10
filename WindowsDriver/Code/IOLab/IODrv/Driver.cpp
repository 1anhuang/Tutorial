#include "IODrv.h"


extern "C" NTSTATUS DriverEntry(
    IN	PDRIVER_OBJECT	pDriverObject,
    IN	PUNICODE_STRING	pRegistryPath
)
{
    DbgPrint(TEXT("[DriverEntry INFO]: Enter"));
    NTSTATUS status;

    pDriverObject->DriverUnload = IODrvUnload;
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = \
        pDriverObject->MajorFunction[IRP_MJ_CLOSE] = \
        pDriverObject->MajorFunction[IRP_MJ_READ] = \
        pDriverObject->MajorFunction[IRP_MJ_WRITE] = IODrvDefaultDispatch;

    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IODrvIOControl;

    status = CreateDevice(pDriverObject);

    DbgPrint(TEXT("[DriverEntry INFO]: End"));
    return status;
}



NTSTATUS CreateDevice(
    IN	PDRIVER_OBJECT	pDriverObject
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT pDevObj;
    PDEVICE_EXTENSION pDevExt = NULL;

    UNICODE_STRING devName;
    RtlInitUnicodeString(&devName, L"\\Device\\IODrv");

    status = IoCreateDevice(
        pDriverObject,
        sizeof(DEVICE_EXTENSION),
        &devName,
        FILE_DEVICE_UNKNOWN,
        0,
        TRUE,
        &pDevObj
    );
    if (!NT_SUCCESS(status))
    {
        DbgPrint(TEXT("IoCreateDevice == %08X"), status);
        return status;
    }


    pDevObj->Flags |= DO_BUFFERED_IO;
    pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
    pDevExt->pDevice = pDevObj;
    pDevExt->ustrDeviceName = devName;

    // create symbolic name
    UNICODE_STRING symLinkName;
    RtlInitUnicodeString(&symLinkName, L"\\??\\IODrv");
    pDevExt->ustrSymLinkName = symLinkName;
    status = IoCreateSymbolicLink(&symLinkName, &devName);
    if (!NT_SUCCESS(status))
    {
        DbgPrint(TEXT("IoCreateSymbolicLink == %08X"), status);
        IoDeleteDevice(pDevObj);
        return status;
    }

    return STATUS_SUCCESS;
}


VOID IODrvUnload(
    IN	PDRIVER_OBJECT pDriverObject
)
{
    DbgPrint(TEXT("[IODrvUnload INFO]: Enter"));
    
    
    PDEVICE_OBJECT pDevObj = pDriverObject->DeviceObject;

    while (pDevObj != NULL)
    {
        PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
        UNICODE_STRING symLinkName = devExt->ustrSymLinkName;
        IoDeleteSymbolicLink(&symLinkName);
        pDevObj = pDevObj->NextDevice;
        IoDeleteDevice(devExt->pDevice);
    }


    DbgPrint(TEXT("[IODrvUnload INFO]: End"));
}


NTSTATUS IODrvDefaultDispatch(
    IN PDEVICE_OBJECT   pDevObj,
    IN PIRP             pIRP
)
{
    DbgPrint(TEXT("[IODrvDefaultDispatch INFO]: Enter"));

    pIRP->IoStatus.Status = STATUS_SUCCESS;
    pIRP->IoStatus.Information = 0;
    IoCompleteRequest(pIRP, IO_NO_INCREMENT);

    DbgPrint(TEXT("[IODrvDefaultDispatch INFO]: End"));
    return STATUS_SUCCESS;
}


NTSTATUS IODrvIOControl(
    IN PDEVICE_OBJECT   pDevObj,
    IN PIRP             pIRP
)
{
    DbgPrint(TEXT("[IODrvIOControl INFO]: Enter"));
    
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIRP);
    ULONG ulInLen = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG ulOutLen = stack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG ulCode = stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG ulInfo = 0;
    PVOID pvIOBuf = (PULONG)pIRP->AssociatedIrp.SystemBuffer;

    PORT_STRUCT portStruct = { 0 };


    switch (ulCode)
    {
    case IOCTL_READ_PORT:

        DbgPrint("[IODrvIOControl INFO]: IOCTL_READ_PORT Enter");

        if (pvIOBuf)
        {
            memcpy_s(&portStruct, sizeof(PORT_STRUCT), pvIOBuf, min(sizeof(PORT_STRUCT), ulInLen));

            DbgPrint("[IODrvIOControl INFO]: size=%d, port=%04X", portStruct.ucSize, portStruct.wPort);

            switch (portStruct.ucSize)
            {
            case 1:
                portStruct.dwPortVal = READ_PORT_UCHAR((PUCHAR)portStruct.wPort);
                ulInfo = 4;
                status = STATUS_SUCCESS;
                break;
            case 2:
                portStruct.dwPortVal = READ_PORT_USHORT((PUSHORT)portStruct.wPort);
                ulInfo = 4;
                status = STATUS_SUCCESS;
                break;
            case 4:
                portStruct.dwPortVal = READ_PORT_ULONG((PULONG)portStruct.wPort);
                ulInfo = 4;
                status = STATUS_SUCCESS;
                break;
            default:
                break;
            }

            (ULONG)*(ULONG*)pvIOBuf = portStruct.dwPortVal;
        }

        DbgPrint("[IODrvIOControl INFO]: IOCTL_READ_PORT End");

        break;
    case IOCTL_WRITE_PORT:

        if (pvIOBuf)
        {
            memcpy_s(&portStruct, sizeof(PORT_STRUCT), pvIOBuf, min(sizeof(PORT_STRUCT), ulInLen));

            switch (portStruct.ucSize)
            {
            case 1:
                WRITE_PORT_UCHAR((PUCHAR)portStruct.wPort, (UCHAR)portStruct.dwPortVal);
                status = STATUS_SUCCESS;
                break;
            case 2:
                WRITE_PORT_USHORT((PUSHORT)portStruct.wPort, (USHORT)portStruct.dwPortVal);
                status = STATUS_SUCCESS;
                break;
            case 4:
                WRITE_PORT_ULONG((PULONG)portStruct.wPort, (ULONG)portStruct.dwPortVal);
                status = STATUS_SUCCESS;
                break;
            default:
                break;
            }
        }
        break;
    }

    pIRP->IoStatus.Status = status;
    pIRP->IoStatus.Information = ulInfo;

    IoCompleteRequest(pIRP, IO_NO_INCREMENT);

    DbgPrint(TEXT("[IODrvIOControl INFO]: End"));
    return status;
}