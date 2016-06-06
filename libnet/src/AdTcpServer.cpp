/***************************************
*	Description: tcp Server module
*	Author: wilson
*	Date: 2015/08/14
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include "AdTcpServer.h"

AdTcpServer::AdTcpServer()
{
	m_iServerSocket = AD_FAILURE;
}

AdTcpServer::~AdTcpServer()
{
	TcpServerClose();
}

int AdTcpServer::TcpServerGetSocket()
{
	return m_iServerSocket;
}


void AdTcpServer::TcpServerClose()
{
	map<UINT, AdTcp *>::iterator  iter;

	for(iter = m_mSession.begin(); iter != m_mSession.end(); iter ++)
	{
		delete iter->second;
	}
	m_mSession.clear();
	
	if(m_iServerSocket != AD_FAILURE)
	{
		close(m_iServerSocket);
		m_iServerSocket = AD_FAILURE;
	}
}

void AdTcpServer::TcpServerDisconnect(AdTcp * pTcp)
{
	map<UINT, AdTcp *>::iterator  iter;
	iter = m_mSession.find(pTcp->GetSocket());
	if(iter != m_mSession.end())
	{
		delete iter->second;
		m_mSession.erase(iter);
	}
}

int AdTcpServer::TcpServerInit( int iPort,  bool  bBlock, bool bReusePort)
{
	int ret;
	
	m_iServerSocket=socket(AF_INET,SOCK_STREAM,0);
	if(m_iServerSocket == AD_FAILURE)
	{
		AD_ERROR("Socket init Error\n");
		 return AD_FAILURE;
	}

	if(bReusePort){
		int optval = 1;
		ret = ::setsockopt(m_iServerSocket, SOL_SOCKET, SO_REUSEADDR,
							   &optval, static_cast<socklen_t>(sizeof optval));
		optval = 1;
		ret = ::setsockopt(m_iServerSocket, SOL_SOCKET, SO_REUSEPORT,
							   &optval, static_cast<socklen_t>(sizeof optval));
	}

	bzero(&m_stAddr,sizeof(struct sockaddr_in));
	m_stAddr.sin_family=AF_INET;
	m_stAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	m_stAddr.sin_port=htons(iPort);

	ret = bind(m_iServerSocket,(struct sockaddr *)(&m_stAddr),sizeof(struct sockaddr));
	if(ret != AD_SUCCESS)
	{
		 AD_ERROR("Socket bind Error\n");
		 TcpServerClose();
		 return AD_FAILURE;
	}

	ret= listen(m_iServerSocket, 2048);
	if(ret != AD_SUCCESS)
	{
		 AD_ERROR("Socket listen Error\n");
		 TcpServerClose();
		 return AD_FAILURE;
	}

	if(bBlock)
		return AD_SUCCESS;

	if (AD_FAILURE == fcntl(m_iServerSocket, F_SETFL, fcntl(m_iServerSocket, F_GETFL, 0) | O_NONBLOCK)) 
	{
		AD_ERROR("Fcntl error\n");
		return AD_FAILURE;
	}
	
	return AD_SUCCESS;
}


AdTcp * AdTcpServer::TcpServerAccept()
{
	AdTcp *pTcp = new AdTcp;

	int ret  = pTcp->Accept(m_iServerSocket);
	if(ret == AD_FAILURE)
	{
		 AD_ERROR("Socket Accept Error\n");
		 delete pTcp;
		 return NULL;
	}
	m_mSession[pTcp->GetSocket()] = pTcp;
	return  pTcp;
}
