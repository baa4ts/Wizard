#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <thread>
#include <atomic>
#include "include/server.hpp"

SERVICE_STATUS_HANDLE g_ServiceHandle = nullptr;
SERVICE_STATUS g_ServiceStatus = {0};

// Variables
std::atomic<bool> Interruptor{false};

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI ServiceCtrlHandler(DWORD control);

int main()
{
    SERVICE_TABLE_ENTRY serviceTable[] = {
        {(LPSTR) "Wizard", (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}};

    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        return 1;
    }

    return 0;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    g_ServiceHandle = RegisterServiceCtrlHandler(TEXT("Wizard"), ServiceCtrlHandler);
    if (!g_ServiceHandle)
    {
        return;
    }

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

    // Atencion //
    Interruptor.store(true, std::memory_order_seq_cst);
    //////////////

    /*
     * Hilo para la coneccion TCP
     */

    std::thread ServidorTCP(Comunicacion::iniciarServidor);
    ServidorTCP.detach();

    while (Interruptor.load(std::memory_order_seq_cst))
    {
        Sleep(100);
    }

    // Atencion //
    Interruptor.store(false, std::memory_order_seq_cst);
    //////////////

    g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD control)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:
        // Atencion //
        Interruptor.store(false, std::memory_order_seq_cst);
        //////////////

        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);
        break;
    default:
        break;
    }
}
