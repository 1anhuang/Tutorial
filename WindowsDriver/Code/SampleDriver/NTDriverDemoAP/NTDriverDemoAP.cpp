// NTDriverDemoAP.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <Windows.h>
#include <shlwapi.h>
#include <atlconv.h>

#pragma comment(lib,"shlwapi.lib")  


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

    //GetFullPathName(tszFilePath, 256, tszImagePath, NULL);
    printf("tszFilePath=%ls\n", tszFilePath);
    printf("tszSvcName=%ls\n", tszSvcName);

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


int main()
{
    TCHAR tszExePath[256] = L"";
    char srvName[256] = "HelloNTDriver";
    char srvPath[256] = "";
    HANDLE hDevice = NULL;
    DWORD dwWrite = 0, dwRead = 0;


    GetModuleFileName(NULL, tszExePath, 256);
    PathRemoveFileSpec(tszExePath);
    sprintf_s(srvPath, "%ws\\%s.sys", tszExePath, srvName);


    UnloadNTDriver(srvName);
    // Install driver as service.
    if (!LoadNTDriver(srvName, srvPath))
    {
        printf("Failed to load driver service.\n");
        goto Exit;
    }
    printf("Load NT driver successfully.\n");


    // Open driver (symbolic name)
    hDevice = CreateFile(
        L"\\\\.\\HelloNTDriver",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hDevice == NULL || hDevice == INVALID_HANDLE_VALUE)
    {
        printf("CreateDevice error=%08X\n", GetLastError());
    }
    else
    {
        // ---------------------------------------------------
        // Read/Write Driver
        // ---------------------------------------------------
        if (!WriteFile(hDevice, NULL, 0, &dwWrite, NULL))
        {
            printf("WriteFile error=%08X\n", GetLastError());
        }
        if (!ReadFile(hDevice, NULL, 0, &dwRead, NULL))
        {
            printf("ReadFile error=%08X\n", GetLastError());
        }

        // Close driver
        CloseHandle(hDevice);
    }
 

    StopNTService(srvName);
    // Uninstall service.
    if (!UnloadNTDriver(srvName))
    {
        printf("Failed to unload driver service.\n");
        goto Exit;
    }
    printf("Unload NT driver successfully.\n");
    
    
Exit:
    system("pause");
    return 0;
}

