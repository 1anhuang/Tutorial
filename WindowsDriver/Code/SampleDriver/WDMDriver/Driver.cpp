#include "Driver.h"

#pragma INITCODE
extern "C" NTSTATUS DriverEntry(
	IN	PDRIVER_OBJECT	pDriverObject,
	IN	PUNICODE_STRING	pRegistryPath
)
{
	NTSTATUS status;
	DbgPrint(TEXT("[DriverEntry INFO]: Enter\n"));


	// Set routine callback
	pDriverObject->DriverUnload = HelloWDMUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDispatchRoutine;


	// Create Device
	status = CreateDevice(pDriverObject);


	DbgPrint(TEXT("[DriverEntry INFO]: End\n"));

	return status;
}


#pragma INITCODE
NTSTATUS CreateDevice(
	IN	PDRIVER_OBJECT	pDriverObject
)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyWDMDriver");

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
	RtlInitUnicodeString(&symLinkName, L"\\??\\HelloWDMDriver");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}

	return STATUS_SUCCESS;
}


#pragma PAGECODE
VOID HelloWDMUnload(
	IN	PDRIVER_OBJECT pDriverObject
)
{
	DbgPrint(TEXT("[HelloWDMUnload INFO]: Enter"));
	PDEVICE_OBJECT pNextObj;

	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj != NULL)
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextObj->DeviceExtension;

		// Delete symbolic link
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;

		// Delete device
		IoDeleteDevice(pDevExt->pDevice);
	}
}


#pragma PAGECODE
NTSTATUS HelloWDMDispatchRoutine(
	IN	PDEVICE_OBJECT	pDevObj,
	IN	PIRP			pIRP
)
{
	DbgPrint(TEXT("[HelloWDMDispatchRoutine INFO]: Enter"));
	NTSTATUS status = STATUS_SUCCESS;

	//complete IRP
	pIRP->IoStatus.Status = status;
	pIRP->IoStatus.Information = 0;

	IoCompleteRequest(pIRP, IO_NO_INCREMENT);
	DbgPrint(TEXT("[HelloWDMDispatchRoutine INFO]: Leave"));
	return status;
}