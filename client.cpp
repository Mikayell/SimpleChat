#include "common_headers.h"
#include "client.h"

using boost::asio::ip::tcp;
using namespace boost::asio;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Use: client <nickname> <localhost> <port>\n";
            return 1;
        }

        io_context context;
        tcp::resolver resolver(context);
        tcp::resolver::query query(argv[2], argv[3]);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        std::array<char, MAX_NICKNAME_SIZE> nickname;
        strcpy(nickname.data(), argv[1]);

        Client client(nickname, context, iterator);

        std::thread mainThread([&context](){context.run();});

        std::array<char, MAX_PACK_SIZE> msg;

        while (true)
        {
            memset(msg.data(), '\0', msg.size());
            if (!std::cin.getline(msg.data(), MAX_PACK_SIZE - PADDING - MAX_NICKNAME_SIZE))
            {
                std::cin.clear();
            }
            client.Write(msg);
        }

        client.Close();

        mainThread.join();

    } catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}


Client::Client(const std::array<char, MAX_NICKNAME_SIZE>& nickname,
            io_context& context,
            tcp::resolver::iterator ep_iterator) :
            _context(context), _socket(context)
{
    strcpy(_nickname.data(), nickname.data());
    memset(_readMsg.data(), '\0', MAX_PACK_SIZE);
    async_connect(_socket, ep_iterator, [this](const boost::system::error_code& error, const ip::tcp::resolver::iterator& ep_iterator){
        onConnect(error);
    });
    
}

void Client::Write(const std::array<char, MAX_PACK_SIZE>& msg)
{
    _context.post([msg, this](){onWrite(msg);});
}

void Client::Close()
{
    _context.post([this](){ onClose();});
}
void Client::onConnect(const boost::system::error_code& error)
{
    if (!error)
    {
        async_write(_socket, buffer(_nickname, _nickname.size()),
                    [this](const boost::system::error_code& error, std::size_t bytes){ readHandler(error);});
    }
}
void Client::readHandler(const boost::system::error_code& error)
{
    std::cout << _readMsg.data() << std::endl;
    if (!error)
    {
        async_read(_socket, buffer(_readMsg, _readMsg.size()),
                    [this](const boost::system::error_code& error, std::size_t bytes){ readHandler(error);});
    } else
    {
        onClose();
    }
}

void Client::onWrite(std::array<char, MAX_PACK_SIZE> msg)
{
    bool write_in_progress = !_writeQueue.empty();
    _writeQueue.push_back(msg);
    if (!write_in_progress)
    {
        async_write(_socket, buffer(_writeQueue.front(), _writeQueue.front().size()),
                    [this](const boost::system::error_code& error, std::size_t bytes){ writeHandler(error);});
    }
}

void Client::writeHandler(const boost::system::error_code& error)
{
    if (!error)
    {
        _writeQueue.pop_front();
        if (!_writeQueue.empty())
        {
            async_write(_socket, buffer(_writeQueue.front(), _writeQueue.front().size()),
                        [this](const boost::system::error_code& error, std::size_t bytes){ writeHandler(error);});
        }
    } else
    {
        onClose();
    }
}

void Client::onClose()
{
    _socket.close();
}