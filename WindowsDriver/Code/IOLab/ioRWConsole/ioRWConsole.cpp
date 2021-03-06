// ioRWConsole.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")  

#pragma region IOCTL Code

#define IOCTL_READ_PORT         CTL_CODE( \
                                FILE_DEVICE_UNKNOWN, \
                                0x800, \
                                METHOD_BUFFERED, \
                                FILE_ANY_ACCESS)


#define IOCTL_WRITE_PORT        CTL_CODE( \
                                FILE_DEVICE_UNKNOWN, \
                                0x800 + 1, \
                                METHOD_BUFFERED, \
                                FILE_ANY_ACCESS)


#define IOCTL_READ_REGISTER     CTL_CODE( \
                                FILE_DEVICE_UNKNOWN, \
                                0x800 + 2, \
                                METHOD_BUFFERED, \
                                FILE_ANY_ACCESS)


#define IOCTL_WRITE_REGISTER    CTL_CODE( \
                                FILE_DEVICE_UNKNOWN, \
                                0x800 + 3, \
                                METHOD_BUFFERED, \
                                FILE_ANY_ACCESS)

#pragma endregion



// Port
typedef struct _PORT_STRUCT
{
    USHORT  wPort;
    ULONG   dwPortVal;
    UCHAR   ucSize;
} PORT_STRUCT, PPORT_STRUCT;


// Register
union REG_DATA
{
    ULONG32 ulData;
    USHORT  usData;
    UCHAR   ucData;
};

typedef struct _REG_STRUCT
{
    ULONG64 ullAddress;
    REG_DATA reg;
    UCHAR   ucSize;

} REG_STRUCT, *PREG_STRUCT;



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


ULONG ReadPortUCHAR(
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


ULONG WritePortUCHAR(
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


ULONG ReadRegisterUCHAR(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddr,
    __out   UCHAR   *pucData
)
{
    ULONG ulVal = 0, ulLen = 0;
    REG_STRUCT regStruct{ 0 };

    regStruct.ullAddress = ullAddr;
    regStruct.ucSize = 1;

    if (!DeviceIoControl(hDrv, IOCTL_READ_REGISTER, &regStruct, sizeof(REG_STRUCT), &regStruct, sizeof(REG_STRUCT), &ulLen, NULL))
    {
        return GetLastError();
    }


    *pucData = regStruct.reg.ucData;
    return 0;
}


ULONG ReadRegisterUSHORT(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddr,
    __out   USHORT  *pusData
)
{
    ULONG ulVal = 0, ulLen = 0;
    REG_STRUCT regStruct{ 0 };

    regStruct.ullAddress = ullAddr;
    regStruct.ucSize = 2;

    if (!DeviceIoControl(hDrv, IOCTL_READ_REGISTER, &regStruct, sizeof(REG_STRUCT), &regStruct, sizeof(REG_STRUCT), &ulLen, NULL))
    {
        return GetLastError();
    }


    *pusData = regStruct.reg.usData;
    return 0;
}


ULONG ReadRegisterULONG(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddr,
    __out   ULONG   *pulData
)
{
    ULONG ulVal = 0, ulLen = 0;
    REG_STRUCT regStruct{ 0 };

    regStruct.ullAddress = ullAddr;
    regStruct.ucSize = 4;


    if (!DeviceIoControl(hDrv, IOCTL_READ_REGISTER, &regStruct, sizeof(REG_STRUCT), &regStruct, sizeof(REG_STRUCT), &ulLen, NULL))
    {
        return GetLastError();
    }


    *pulData = regStruct.reg.ulData;
    return 0;
}



ULONG WriteRegisterUCHAR(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddr,
    __out   UCHAR   ucData
)
{
    ULONG ulVal = 0, ulLen = 0;
    REG_STRUCT regStruct{ 0 };

    regStruct.ullAddress = ullAddr;
    regStruct.ucSize = 1;
    regStruct.reg.ucData = ucData;

    if (!DeviceIoControl(hDrv, IOCTL_WRITE_REGISTER, &regStruct, sizeof(REG_STRUCT), NULL, 0, &ulLen, NULL))
    {
        return GetLastError();
    }

    return 0;
}


ULONG WriteRegisterUSHORT(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddr,
    __out   USHORT  usData
)
{
    ULONG ulVal = 0, ulLen = 0;
    REG_STRUCT regStruct{ 0 };

    regStruct.ullAddress = ullAddr;
    regStruct.ucSize = 2;
    regStruct.reg.usData = usData;

    if (!DeviceIoControl(hDrv, IOCTL_WRITE_REGISTER, &regStruct, sizeof(REG_STRUCT), NULL, 0, &ulLen, NULL))
    {
        return GetLastError();
    }

    return 0;
}


ULONG ReadRegisterULONG(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddr,
    __out   ULONG   ulData
)
{
    ULONG ulVal = 0, ulLen = 0;
    REG_STRUCT regStruct{ 0 };

    regStruct.ullAddress = ullAddr;
    regStruct.ucSize = 4;
    regStruct.reg.ulData = ulData;


    if (!DeviceIoControl(hDrv, IOCTL_READ_REGISTER, &regStruct, sizeof(REG_STRUCT), NULL, 0, &ulLen, NULL))
    {
        return GetLastError();
    }


    return 0;
}

#pragma endregion


bool ReadPortData(
    __in    HANDLE  hDrv,
    __in    WORD    wAddress,
    __in    int     nCount,
    __in    int     nSize,
    __out   UCHAR   *pBuff,
    __out   int     *pnLength
    )
{
    if (pBuff == NULL || pnLength == NULL)
    {
        return false;
    }

    *pnLength = 0;
    ULONG ulRet = 0;
    for (int i = 0; *pnLength < nSize && i < nCount; i++)
    {
        ulRet = ReadPortUCHAR(hDrv, wAddress + i, pBuff + *pnLength);
        if (ulRet != 0)
        {
            printf("[ReadPortData INFO]: ReadPortUCHAR == 0x%08X", ulRet);
            return false;
        }

        *pnLength = *pnLength + 1;
    }

    return true;
}


bool ReadRegisterData(
    __in    HANDLE  hDrv,
    __in    ULONG64 ullAddress,
    __in    int     nCount,
    __in    int     nSize,
    __out   UCHAR   *pBuff,
    __out   int     *pnLength
)
{
    if (pBuff == NULL || pnLength == NULL)
    {
        return false;
    }

    *pnLength = 0;
    ULONG ulRet = 0;
    for (int i = 0; *pnLength < nSize && i < nCount; i++)
    {
        ulRet = ReadRegisterUCHAR(hDrv, ullAddress + i, pBuff + *pnLength);
        if (ulRet != 0)
        {
            printf("[ReadPortData INFO]: ReadRegisterData == 0x%08X", ulRet);
            return false;
        }

        *pnLength = *pnLength + 1;
    }

    return true;
}


void DisplayData(
    __in    ULONG64 ullAddress,
    __in    UCHAR   *pData,
    __in    int     nLength
)
{
    printf("I/O Port 0x%08X:\n", ullAddress);
    for (int i = 0; i < nLength; i++)
    {
        if (i != 0 && i % 16 == 0)
        {
            printf("\n");
        }

        printf("%02X ", pData[i]);
    }

    printf("\n\n");
}

int main()
{
    char szDrvName[256] = "\\\\.\\IODrv";
    char szSrvPath[256] = "";
    char szSrvName[256] = "IODrv";
    TCHAR tszExePath[256] = L"";


    HANDLE hDrv = NULL;

    WORD wPortAddress = 0;
    UCHAR dataBuffer[1024 * 8] = "";
    int nLength = 0;

    ULONG ulRet = 0;
    UCHAR ucData = 0;
    ULONG ulData = 0;


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


    // Read port data
    if (ReadPortData(hDrv, 0xF000, 256, sizeof(dataBuffer), dataBuffer, &nLength))
    {
        DisplayData(0xF000, dataBuffer, nLength);
    }
    else
    {
        printf("ReadPortData == FALSE\n");
    }


    memset(dataBuffer, 0, sizeof(dataBuffer));
    // Read register data
    if (ReadRegisterData(hDrv, 0xF0215000, 256, sizeof(dataBuffer), dataBuffer, &nLength))
    {
        DisplayData(0xF0215000, dataBuffer, nLength);
    }
    else
    {
        printf("ReadRegisterData == FALSE\n");
    }

    CloseDriver(hDrv);
    StopNTService(szSrvName);
    UnloadNTDriver(szSrvName);

Exit:

    system("pause");
    return 0;
}

