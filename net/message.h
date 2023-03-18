#ifndef MESSAGE_H
#define MESSAGE_H

#include "common_headers.h"

class MessageHeader
{
public:
    int id;
    int size;
};

class Message
{
public:
    MessageHeader header;
    std::vector<uint8_t> body;

    size_t size() const
	{
		return body.size();
	}


private:
    
};
#endif