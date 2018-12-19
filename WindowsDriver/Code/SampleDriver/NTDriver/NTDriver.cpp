#include "NTDriver.h"


extern "C" NTSTATUS DriverEntry(
	IN	PDRIVER_OBJECT	pDriverObject,
	IN	PUNICODE_STRING	pRegistryPath
)
{
	NTSTATUS status;
	DbgPrint(TEXT("[DriverEntry INFO]: Enter\n"));


	// Set routine callback
	pDriverObject->DriverUnload = HelloNTUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloNTDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloNTDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloNTDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloNTDispatchRoutine;


	// Create Device
	status = CreateDevice(pDriverObject);


	DbgPrint(TEXT("[DriverEntry INFO]: End\n"));

	return status;
}



NTSTATUS CreateDevice(
	IN	PDRIVER_OBJECT	pDriverObject
)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyNTDriver");

	// Create device
	status = IoCreateDevice(pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		TRUE,
		&pDevObj
	);

	if (!NT_SUCCESS(status))
	{
		return status;
	}


	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;

	// Create symbolic name
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\HelloNTDriver");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}

	return STATUS_SUCCESS;
}


//#pragma PAGECODE
VOID HelloNTUnload(
    IN	PDRIVER_OBJECT pDriverObject
)
{
    DbgPrint(TEXT("[HelloNTUnload INFO]: Enter"));


    PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDriverObject->DeviceObject->DeviceExtension;

    // Delete symbolic link
    UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
    IoDeleteSymbolicLink(&pLinkName);
    IoDeleteDevice(pDriverObject->DeviceObject);
}


//#pragma PAGECODE
NTSTATUS HelloNTDispatchRoutine(
	IN	PDEVICE_OBJECT	pDevObj,
	IN	PIRP			pIRP
)
{
	DbgPrint(TEXT("[HelloNTDispatchRoutine INFO]: Enter"));
	NTSTATUS status = STATUS_SUCCESS;

	//complete IRP
	pIRP->IoStatus.Status = status;
	pIRP->IoStatus.Information = 0;

	IoCompleteRequest(pIRP, IO_NO_INCREMENT);
	DbgPrint(TEXT("[HelloNTDispatchRoutine INFO]: Leave"));
	return status;
}