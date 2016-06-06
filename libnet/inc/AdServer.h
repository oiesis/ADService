/***************************************
*	Description: server module, detect server and session status
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_SERVER_H__
#define __AD_SERVER_H__

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include "AdTcp.h"
#include "AdTcpServer.h"
#include "AdSession.h"
#include "AdGlobal.h"
#include "AdNetBase.h"

class AdServer:public AdNetBase
{
public:
	AdServer();
	~AdServer();
	int AdServerInit(int iPort, int msgRead, int msgWrite);
private:
	AdTcpServer   m_ctTcpServer;
};

#endif
