// ioRWConsole.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")  

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



#pragma region Install / Uninstall Driver

BOOL LoadNTDriver(const char *pszSvcName, const char *pszFilePath)
{
    BOOL bRet = FALSE;
    DWORD dwLastError = 0;
    TCHAR tszSvcName[256] = L"", tszFilePath[256] = L"", tszImagePath[256] = L"";;
    SC_HANDLE hSvcMgr = NULL, hSvcNT = NULL;

#pragma warning(push)
#pragma warning(disable: 4477)

    swprintf_s(tszSvcName, 256, L"%hs", pszSvcName);
    swprintf_s(tszFilePath, 256, L"%hs", pszFilePath);

#pragma warning(pop)

    hSvcMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSvcMgr == NULL)
    {
        printf("Failed to open 'SC Manager'. error=%d\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }


    hSvcNT = CreateService(
        hSvcMgr,
        tszSvcName,
        tszSvcName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        tszFilePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (hSvcNT == NULL)
    {
        dwLastError = GetLastError();
        if (dwLastError != ERROR_IO_PENDING && dwLastError != ERROR_SERVICE_EXISTS)
        {
            printf("Failed to create service. error=%08X\n", dwLastError);
            bRet = FALSE;
            goto Exit;
        }
        else
        {
            if (dwLastError == ERROR_IO_PENDING)
            {
                printf("Failed to create service. ERROR_IO_PENDING\n");
            }
            printf("Failed to create service. ERROR_IO_PENDING or ERROR_SERVICE_EXISTS\n");
            hSvcNT = OpenService(hSvcMgr, tszSvcName, SERVICE_ALL_ACCESS);
            if (hSvcNT == NULL)
            {
                printf("Failed to open service. error=%08X\n", dwLastError);
                bRet = FALSE;
                goto Exit;
            }
            else
            {
                printf("Open service successfully.\n");
            }
        }
    }


    bRet = StartService(hSvcNT, NULL, NULL);
    if (!bRet)
    {
        dwLastError = GetLastError();
        if (dwLastError != ERROR_SERVICE_ALREADY_RUNNING)
        {
            printf("Failed to start service. error=%08X\n", dwLastError);
            bRet = FALSE;
            goto Exit;
        }
        else
        {
            printf("Service is already running.\n");
        }
    }

Exit:
    if (hSvcMgr)
    {
        CloseServiceHandle(hSvcMgr);
    }
    if (hSvcNT)
    {
        CloseServiceHandle(hSvcNT);
    }
    return bRet;
}


BOOL StopNTService(const char *pszSrvName)
{
    BOOL bRet = FALSE;
    DWORD dwLastError = 0;
    TCHAR tszSrvName[256] = L"", tszFilePath[256] = L"", tszImagePath[256] = L"";;
    SC_HANDLE hSvcMgr = NULL, hSvcNT = NULL;
    SERVICE_STATUS svcStatus;


#pragma warning(push)
#pragma warning(disable: 4477)

    swprintf_s(tszSrvName, L"%hs", pszSrvName);

#pragma warning(pop)



    hSvcMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSvcMgr == NULL)
    {
        printf("Failed to open 'SC Manager'. error=%d\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }


    hSvcNT = OpenService(hSvcMgr, tszSrvName, SERVICE_ALL_ACCESS);
    if (hSvcNT == NULL)
    {
        printf("Failed to open service. error=%08X\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }


    if (!ControlService(hSvcNT, SERVICE_CONTROL_STOP, &svcStatus))
    {
        printf("Failed to stop service. error=%d\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }

Exit:
    if (hSvcMgr)
    {
        CloseServiceHandle(hSvcMgr);
    }
    if (hSvcNT)
    {
        CloseServiceHandle(hSvcNT);
    }
    return bRet;
}


BOOL UnloadNTDriver(const char *pszSrvName)
{
    BOOL bRet = FALSE;
    DWORD dwLastError = 0;
    TCHAR tszSrvName[256] = L"", tszFilePath[256] = L"", tszImagePath[256] = L"";;
    SC_HANDLE hSvcMgr = NULL, hSvcNT = NULL;
    SERVICE_STATUS svcStatus;

#pragma warning(push)
#pragma warning(disable: 4477)

    swprintf_s(tszSrvName, L"%hs", pszSrvName);

#pragma warning(pop)


    hSvcMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSvcMgr == NULL)
    {
        printf("Failed to open 'SC Manager'. error=%d\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }


    hSvcNT = OpenService(hSvcMgr, tszSrvName, SERVICE_ALL_ACCESS);
    if (hSvcNT == NULL)
    {
        printf("Failed to open service. error=%08X\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }


    if (!DeleteService(hSvcNT))
    {
        printf("Failed to delete service. error=%08X\n", GetLastError());
        bRet = FALSE;
        goto Exit;
    }

    bRet = TRUE;


Exit:
    if (hSvcMgr)
    {
        CloseServiceHandle(hSvcMgr);
    }
    if (hSvcNT)
    {
        CloseServiceHandle(hSvcNT);
    }
    return bRet;
}

#pragma endregion


#pragma region Driver Access

ULONG OpenDriver(
    __in    char    *szFileName,
    __out   HANDLE  *hDrv
)
{
    TCHAR tszFileName[256] = L"";

#pragma warning(push)
#pragma warning(disable: 4477)
    
    swprintf_s(tszFileName, L"%hs", szFileName);

#pragma warning(pop)

    *hDrv = CreateFile(
        tszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FALSE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hDrv == NULL || hDrv == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile == NULL || hDrv == INVALID_HANDLE_VALUE \n");
        return GetLastError();
    }


    return 0;
}


ULONG CloseDriver(
    __in    HANDLE  hDrv
)
{
    if (hDrv)
    {
        if (!CloseHandle(hDrv))
        {
            return GetLastError();
        }
    }

    return 0;
}


ULONG ReadPortBYTE(
    __in    HANDLE  hDrv,
    __in    WORD    wPort,
    __out   UCHAR   *pucPortVal
)
{
    ULONG ulVal = 0, ulLen = 0;
    PORT_STRUCT portStruct = { 0 };
    portStruct.ucSize = 1;
    portStruct.wPort = wPort;

    if (pucPortVal == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else if (!DeviceIoControl(hDrv, IOCTL_READ_PORT, &portStruct, sizeof(PORT_STRUCT), &ulVal, sizeof(ULONG), &ulLen, NULL))
    {
        return GetLastError();
    }

    *pucPortVal = (UCHAR)ulVal;
    return 0;
}


ULONG ReadPortWORD(
    __in    HANDLE  hDrv,
    __in    WORD    wPort,
    __out   WORD   *pwPortVal
)
{
    ULONG ulVal = 0, ulLen = 0;
    PORT_STRUCT portStruct = { 0 };
    portStruct.ucSize = 2;
    portStruct.wPort = wPort;

    if (pwPortVal == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else if (!DeviceIoControl(hDrv, IOCTL_READ_PORT, &portStruct, sizeof(PORT_STRUCT), &ulVal, sizeof(ULONG), &ulLen, NULL))
    {
        return GetLastError();
    }

    *pwPortVal = (WORD)ulVal;
    return 0;
}


ULONG ReadPortULONG(
    __in    HANDLE  hDrv,
    __in    WORD    wPort,
    __out   ULONG   *pulPortVal
)
{
    ULONG ulVal = 0, ulLen = 0;
    PORT_STRUCT portStruct = { 0 };
    portStruct.ucSize = 4;
    portStruct.wPort = wPort;

    if (pulPortVal == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else if (!DeviceIoControl(hDrv, IOCTL_READ_PORT, &portStruct, sizeof(PORT_STRUCT), &ulVal, sizeof(ULONG), &ulLen, NULL))
    {
        return GetLastError();
    }

    *pulPortVal = (ULONG)ulVal;
    return 0;
}


ULONG WritePortBYTE(
    __in    HANDLE  hDrv,
    __in    WORD    wPort,
    __in    UCHAR   ucPortVal
)
{
    ULONG ulVal = 0, ulLen = 0;
    PORT_STRUCT portStruct = { 0 };
    portStruct.ucSize = 1;
    portStruct.wPort = wPort;
    portStruct.dwPortVal = ucPortVal;

if (!DeviceIoControl(hDrv, IOCTL_WRITE_PORT, &portStruct, sizeof(PORT_STRUCT), NULL, 0, &ulLen, NULL))
    {
        return GetLastError();
    }


    return 0;
}


ULONG WritePortWORD(
    __in    HANDLE  hDrv,
    __in    WORD    wPort,
    __in    WORD    wPortVal
)
{
    ULONG ulVal = 0, ulLen = 0;
    PORT_STRUCT portStruct = { 0 };
    portStruct.ucSize = 2;
    portStruct.wPort = wPort;
    portStruct.dwPortVal = wPortVal;

    if (!DeviceIoControl(hDrv, IOCTL_WRITE_PORT, &portStruct, sizeof(PORT_STRUCT), NULL, 0, &ulLen, NULL))
    {
        return GetLastError();
    }


    return 0;
}


ULONG WritePortULONG(
    __in    HANDLE  hDrv,
    __in    WORD    wPort,
    __in    WORD    wPortVal
)
{
    ULONG ulVal = 0, ulLen = 0;
    PORT_STRUCT portStruct = { 0 };
    portStruct.ucSize = 2;
    portStruct.wPort = wPort;
    portStruct.dwPortVal = wPortVal;

    if (!DeviceIoControl(hDrv, IOCTL_WRITE_PORT, &portStruct, sizeof(PORT_STRUCT), NULL, 0, &ulLen, NULL))
    {
        return GetLastError();
    }


    return 0;
}

#pragma endregion

int main()
{
    char szDrvName[256] = "\\\\.\\IODrv";
    char szSrvPath[256] = "";
    char szSrvName[256] = "IODrv";
    WORD wPort = 0;
    TCHAR tszExePath[256] = L"";
    HANDLE hDrv = NULL;
    ULONG ulRet = 0;
    UCHAR ucData = 0;


    GetModuleFileName(NULL, tszExePath, 256);
    PathRemoveFileSpec(tszExePath);
    sprintf_s(szSrvPath, "%ws\\%s.sys", tszExePath, szSrvName);

    StopNTService(szSrvName);
    UnloadNTDriver(szSrvName);
    if (!LoadNTDriver(szSrvName, szSrvPath))
    {
        printf("Failed to load driver service.\n");
        goto Exit;
    }
    

    ulRet = OpenDriver(szDrvName, &hDrv);
    if (ulRet != 0)
    {
        printf("OpenDriver == %08X\n", ulRet);
        goto Exit;
    }
    printf("hDrv = %08X\n", hDrv);

    ulRet = ReadPortBYTE(hDrv, 0xF000, &ucData);
    if (ulRet != 0)
    {
        printf("ReadPortBYTE == %08X\n", ulRet);
    }

    printf("0xF000 => 0x%02X\n", ucData);


    CloseDriver(hDrv);
    StopNTService(szSrvName);
    UnloadNTDriver(szSrvName);

Exit:

    system("pause");
    return 0;
}

