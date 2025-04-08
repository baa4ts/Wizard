#ifndef SERVER_HPP
#define SERVER_HPP

#define ASIO_STANDALONE

#include <winsock2.h>
#include <windows.h>
#include <asio.hpp>
#include <thread>
#include <iostream>

namespace Comunicacion
{

    void manejarSesion(asio::ip::tcp::socket sock)
    {
        char data[4096];

        try
        {
            while (true)
            {
                std::size_t length = sock.read_some(asio::buffer(data));

                if (length == 0)
                {
                    break;
                }

                std::string mensaje(data, length);
                if (mensaje.empty())
                {
                    asio::write(sock, asio::buffer("Mensaje vacío, pibe... \n"));
                }
                else
                {
                    if (mensaje.find("exit") != std::string::npos)
                    {
                        break;
                    }

                    // Elimina la llamada a la utilidad PowerShell
                    std::string msg = "\n - - - - - Inicio - - - - -\n" + mensaje + "\n - - - - - Final  - - - - -\n@bin> ";
                    asio::write(sock, asio::buffer(msg));
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
