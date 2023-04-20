#include "common_headers.h"

using boost::asio::ip::tcp;
using namespace boost::asio;

class Client
{
public:
    Client(const std::array<char, MAX_NICKNAME_SIZE>& nickname,
            io_context& context,
            tcp::resolver::iterator ep_iterator);
    void Write(const std::array<char, MAX_PACK_SIZE>& msg);
    void Close();

private:
    void onConnect(const boost::system::error_code& error);
    void readHandler(const boost::system::error_code& error);
    void onWrite(std::array<char, MAX_PACK_SIZE> msg);
    void writeHandler(const boost::system::error_code& error);
    void onClose();

private:
    io_context& _context;
    tcp::socket _socket;
    std::array<char, MAX_PACK_SIZE> _readMsg;
    std::deque<std::array<char, MAX_PACK_SIZE>> _writeQueue;
    std::array<char, MAX_NICKNAME_SIZE> _nickname;
};