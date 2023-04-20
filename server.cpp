#include "common_headers.h"
#include "timestamp.h"
#include "server.h"
#include "worker_thread.h"
#include "room_participants.h"

using boost::asio::ip::tcp;
using namespace boost::asio;

Server::Server(io_context& context,
           io_context::strand& strand,
           const tcp::endpoint& endpoint)
: _context(context), _strand(strand), _acceptor(context, endpoint)
{
    Run();
}

void Server::Run()
{
    std::shared_ptr<RoomMember> new_participant(new RoomMember(_context, _strand, _room));
    _acceptor.async_accept(new_participant->socket(), _strand.wrap([this, new_participant](const boost::system::error_code& error){
        onAccept(new_participant, error);
    }));
}

void Server::onAccept(std::shared_ptr<RoomMember> new_participant, const boost::system::error_code& error)
{
    if (!error)
    {
        new_participant -> Start();
    }

    Run();
}

int main(int argc, char* argv[])
{
    try
    {
        if(argc < 2)
        {
            std::cerr << "Use: server <port> \n";
            return 1;
        }

        std::shared_ptr<io_context> context(new io_context);
        boost::shared_ptr<io_context::work> work(new io_context::work(*context));
        boost::shared_ptr<io_context::strand> strand(new io_context::strand(*context));

        std::cout << "[" << std::this_thread::get_id() << "]" << "Server started!" << std::endl;

        std::list <std::shared_ptr <Server>> servers;

        for(int i = 1; i < argc; ++i)
        {
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            std::shared_ptr<Server> server(new Server(*context, *strand, endpoint));
            servers.push_back(server);
        }

        boost::thread_group workers;
        for(int i = 0; i < 1; ++i)
        {
            boost::thread * thread = new boost::thread{ &workerThread::run, context};
            workers.add_thread(thread);
        }

        workers.join_all();

    } catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
