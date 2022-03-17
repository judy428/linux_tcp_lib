#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

namespace common {

enum CMD {
	CMD_ERROR = 0
};

struct dataHeader
{
	dataHeader()
	{
		cmd = CMD_ERROR;
		dataLength = sizeof(dataHeader);
	}
	uint32_t cmd;
	uint32_t dataLength;
};

struct dataRecv
{
	dataRecv()
	{
		cmd = CMD_ERROR;
		data = nullptr;
	}
	uint32_t cmd;
	char* data;
};

}

#endif