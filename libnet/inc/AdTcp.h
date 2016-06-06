/***************************************
*	Description: tcp module
*	Author: wilson
*	Date: 2015/08/14
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_TCP_H__
#define __AD_TCP_H__

#include <sstream>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "AdGlobal.h"

class AdTcp
{
public:
	AdTcp();
	~AdTcp();

	int Write(const char* pPacket, int iLength);
	int Read(char* pPacket, int iLength, int wait = 0);
	int Connect(const char * ip, int iPort);
	int Accept(int iServerSocket);
	int  GetSocket();
	int NonBlock();
	int Disconnect();
	void GetIp(std::string & sIp);

private:
	struct sockaddr_in m_stAddr;
	int m_iSocket;
	std::string m_sIp;
	
};
#endif
