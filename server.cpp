#include <iostream>
#include <boost/asio.hpp>
#include "net/connection.h"
#include "net/thread_safe_queue.h"
#include <deque>

using namespace boost::asio;
using namespace boost::system;

class Server
{
public:
    Server(int port)
    : _acceptor(_context, ip::tcp::endpoint(ip::tcp::v4(), port))
    {
        
    }

    virtual ~Server()
    {
        Stop();
        std::cout << "Server stopped!" << std::endl;
    }

    bool Start()
    {
        try
        {
            WaitForConnection();
            _serverThread = std::thread([this](){_context.run();});

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        
        std::cout << "Server started!" << std::endl;
        return true;
    }
    void Stop()
    {
        _context.stop();
        if(_serverThread.joinable())
            _serverThread.join();
    }
    void Update(size_t max = -1)
    {
        size_t count = 0;
        while(count < max && !_connections.empty())
        {
            auto msg = _serverQueue.pop_front();
            MessageAll(msg);
            count++;
        }
    }
    void WaitForConnection()
    {
        _acceptor.async_accept(
        [this](std::error_code ec, ip::tcp::socket socket)
        {
            if(!ec)
            {
                std::cout << "New connection: " << socket.remote_endpoint() << std::endl;
                std::shared_ptr<Connection> con = std::make_shared<Connection>(Connection::owner::server,
                _context, std::move(socket), _serverQueue);
                
                if(canBeAccepted(con))
                {
                    _connections.push_back(std::move(con));
                    _connections.back() -> ConnectToClient(ID++);
                    std::cout << _connections.back() -> getID() << " connection approved" << std::endl;
                }
                else
                {
                    std::cout << "connection denied."
                }
            }
            else
            {
                std::cout << "Connection error: " << ec.message << std::endl;
            }
        });
        WaitForConnection();
    }

    void MessageClient(std::shared_ptr<Connection> client, const std::string& msg)
    {
        if(client && client -> IsConnected())
        {
            client -> Send(msg);
        }
        else
        {
            client.reset();
            _connections.erase(std::remove(_connections.begin(), _connections.end(), client), _connections.end());

        }
    }
    void MessageAll(const std::string& msg, std::shared_ptr<Connection> ignoredClient = nullptr)
    {
        bool invalid = false;

        for(auto& client : _connections)
        {
            if(client && client -> IsConnected() && client != ignoredClient)
            {
                client -> Send(msg);
            }
            else
            {
                client.reset();
                invalid = true;            
            }
        }
        if(invalid)
        _connections.erase(std::remove(_connections.begin(), _connections.end(), nullptr), _connections.end());

    }
private:
    TSQueue<std::string> _serverQueue;
    std::deque<std::shared_ptr<Connection>> _connections;
    io_context _context;
    std::thread _serverThread;
    ip::tcp::acceptor _acceptor;
    unsigned int ID = 1000;

private:
    bool canBeAccepted(std::shared_ptr<Connection> client)
    {

    }
};