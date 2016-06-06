/***************************************
*	Description: server module, detect server and session status
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include "AdServer.h"

AdServer::AdServer()
{
}

AdServer::~AdServer()
{
}

int AdServer::AdServerInit(int iPort, int msgRead, int msgWrite)
{
	int ret;
	ret = AdNetBaseInit(msgRead, msgWrite);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("NetBase Init Fail\n");
		return AD_FAILURE;
	}

	ret	= m_ctTcpServer.TcpServerInit(iPort, false);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Tcp Server Init Fail\n");
		return AD_FAILURE;
	}

	ret = AdNetBaseAddServer(m_ctTcpServer.TcpServerGetSocket());
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Add Server Fail\n");
		return AD_FAILURE;
	}

	return AD_SUCCESS;
}
