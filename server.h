#ifndef SERVER_H
#define SERVER_H

#include "common_headers.h"
#include "timestamp.h"
#include "room_participants.h"

using boost::asio::ip::tcp;

class Server
{
public:
    Server(io_context& context,
           io_context::strand& strand,
           const tcp::endpoint& endpoint);
    void Run();

private:
    void onAccept(std::shared_ptr<RoomMember> new_participant, const boost::system::error_code& error);

private:
    io_context& _context;
    io_context::strand& _strand;
    tcp::acceptor _acceptor;
    ChatRoom _room;
};

#endif
