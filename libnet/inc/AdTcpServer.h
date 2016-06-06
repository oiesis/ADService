/***************************************
*	Description: tcp Server module
*	Author: wilson
*	Date: 2015/08/28
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_TCP_SERVER_H__
#define __AD_TCP_SERVER_H__

#include <sstream>
#include <iostream>
#include <string>
#include <map>


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

#include "AdGlobal.h"
#include "AdTcp.h"

using namespace std;

class AdTcpServer
{
public:
	AdTcpServer();
	~AdTcpServer();

	int TcpServerInit( int iPort,  bool  bBlock = true , bool bReusePort = true);
	AdTcp * TcpServerAccept();
	void TcpServerDisconnect(AdTcp * pTcp);
	int TcpServerGetSocket();

protected:
	void TcpServerClose();
private:
	struct sockaddr_in m_stAddr;
	int m_iServerSocket;
	map<UINT, AdTcp *>  m_mSession;
};

#endif

