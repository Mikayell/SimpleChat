#ifndef ROOM_PARTICIPANTS_H
#define ROOM_PARTICIPANTS_H

#include "common_headers.h"
#include "timestamp.h"

using boost::asio::ip::tcp;
using namespace boost::asio;

class Participant
{
public:
    virtual void onMessage(std::array<char, MAX_PACK_SIZE> & msg) = 0;

public:
    virtual ~Participant() = default;
};

class ChatRoom {
public:
    void Enter(std::shared_ptr<Participant> participant, const std::string & nickname)
    {
        _participants.insert(participant);
        _names[participant] = nickname;
        std::for_each(_recentMsgs.begin(), _recentMsgs.end(),
                      boost::bind(&Participant::onMessage, participant, _1));
    }

    void Leave(std::shared_ptr<Participant> participant)
    {
        _participants.erase(participant);
        _names.erase(participant);
    }

    void Broadcast(std::array<char, MAX_PACK_SIZE>& msg, std::shared_ptr<Participant> participant)
    {
        std::string time = getTime();
        std::string nickname = GetNickname(participant);
        std::array<char, MAX_PACK_SIZE> formatted_msg;

        strcpy(formatted_msg.data(), time.c_str());
        strcat(formatted_msg.data(), nickname.c_str());
        strcat(formatted_msg.data(), msg.data());

        _recentMsgs.push_back(formatted_msg);
        while (_recentMsgs.size() > max_recent_msgs)
        {
            _recentMsgs.pop_front();
        }

        std::for_each(_participants.begin(), _participants.end(),
                      boost::bind(&Participant::onMessage, _1, std::ref(formatted_msg)));
    }

    std::string GetNickname(std::shared_ptr<Participant> participant)
    {
        return _names[participant];
    }

private:
    enum { max_recent_msgs = 100 };
    std::unordered_set<std::shared_ptr<Participant>> _participants;
    std::unordered_map<std::shared_ptr<Participant>, std::string> _names;
    std::deque<std::array<char, MAX_PACK_SIZE>> _recentMsgs;
};

class RoomMember: public Participant,
                    public std::enable_shared_from_this<RoomMember>
{
public:
    RoomMember(io_context& io_context,
                 io_context::strand& strand, ChatRoom& room)
                 : _socket(io_context), _strand(strand), _room(room)
    {
    }

    tcp::socket& socket() { return _socket; }

    void Start()
    {
        async_read(_socket, buffer(_nickname, _nickname.size()),
                    _strand.wrap(boost::bind(&RoomMember::nicknameHandler, shared_from_this(), _1)));
    }

private:
    void onMessage(std::array<char, MAX_PACK_SIZE>& msg)
    {
        bool writing = !_writeQueue.empty();
        _writeQueue.push_back(msg);
        if (!writing)
        {
            async_write(_socket, buffer(_writeQueue.front(), _writeQueue.front().size()),
                        _strand.wrap(boost::bind(&RoomMember::writeHandler, shared_from_this(), _1)));
        }
    }
    void nicknameHandler(const boost::system::error_code& error)
    {
        if (strlen(_nickname.data()) <= MAX_NICKNAME_SIZE - 2)
        {
            strcat(_nickname.data(), ": ");
        }
        else
        {
            _nickname[MAX_NICKNAME_SIZE - 2] = ':';
            _nickname[MAX_NICKNAME_SIZE - 1] = ' ';
        }

        _room.Enter(shared_from_this(), std::string(_nickname.data()));

        async_read(_socket, buffer(_readMsg, _readMsg.size()),
                    _strand.wrap(boost::bind(&RoomMember::readHandler, shared_from_this(), _1)));
    }

    void readHandler(const boost::system::error_code& error)
    {
        if (!error)
        {
            _room.Broadcast(_readMsg, shared_from_this());

            async_read(_socket, buffer(_readMsg, _readMsg.size()),
                        _strand.wrap(boost::bind(&RoomMember::readHandler, shared_from_this(), _1)));
        }
        else
        {
            _room.Leave(shared_from_this());
        }
    }

    void writeHandler(const boost::system::error_code& error)
    {
        if (!error)
        {
            _writeQueue.pop_front();

            if (!_writeQueue.empty())
            {
                async_write(_socket, buffer(_writeQueue.front(), _writeQueue.front().size()),
                            _strand.wrap(boost::bind(&RoomMember::writeHandler, shared_from_this(), _1)));
            }
        }
        else
        {
            _room.Leave(shared_from_this());
        }
    }

    tcp::socket _socket;
    io_context::strand& _strand;
    ChatRoom& _room;
    std::array<char, MAX_NICKNAME_SIZE> _nickname;
    std::array<char, MAX_PACK_SIZE> _readMsg;
    std::deque<std::array<char, MAX_PACK_SIZE> > _writeQueue;
};


#endif