/***************************************
*	Description: base class for net.
*	Author: wilson
*	Date: 2015/07/20
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#include "AdUdp.h"

AdUdp::AdUdp()
{
	memset(&m_stAddr, '\0', sizeof(struct sockaddr_in));
	m_iSocket = AD_FAILURE;
	m_iSocketInit = false;
}

AdUdp::~AdUdp()
{
	closeSocket();
}

void AdUdp::closeSocket()
{
	if(m_iSocketInit)
	{
		close(m_iSocket);
		m_iSocket = AD_FAILURE;
		m_iSocketInit = false;
	}
}

int AdUdp::initSocket(struct sockaddr* pAddr, bool bReUse, bool bBind)
{
	int ret = AD_SUCCESS;
	int iOn = 1;
	
	m_iSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_iSocket <= 0)
	{
		AD_ERROR("sock init error\n");
		return AD_FAILURE;
	}

	if(bReUse)
	{
		ret = setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, &iOn, sizeof(iOn));
		if(ret != AD_SUCCESS) 
		{
			AD_ERROR("setsockeopt error\n");
			closeSocket();
			return ret;
		}
	}

	if(bBind)
	{
		ret = bind(m_iSocket, pAddr, sizeof(struct sockaddr_in));
		if(ret != AD_SUCCESS) 
		{
			AD_ERROR("bind error\n");
			closeSocket();
		}
	}

	if(ret == AD_SUCCESS)
		m_iSocketInit = true;
	return ret;
}

int  AdUdp::UdpInit(void* pAddr)
{
	memcpy(&m_stAddr, pAddr, sizeof(struct sockaddr_in));
	return initSocket((struct sockaddr *)(&m_stAddr), false);
}

int  AdUdp::UdpInit(char * pUrl, int iPort)
{
	if(pUrl == NULL)
	{
		m_stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		m_stAddr.sin_addr.s_addr = inet_addr(pUrl);
	}
	m_stAddr.sin_port = htons(iPort);
	m_stAddr.sin_family = AF_INET;
	if(pUrl == NULL)
		return initSocket((struct sockaddr *)(&m_stAddr), false);
	else
		return initSocket((struct sockaddr *)(&m_stAddr), false, false);
}

int  AdUdp::UdpMultiInit(char * pUrl, int iPort)
{
	int ret;
		
	if(pUrl != NULL)
	{
		m_stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		m_stAddr.sin_addr.s_addr = inet_addr(pUrl);
	}
	m_stAddr.sin_port = htons(iPort);
	m_stAddr.sin_family = AF_INET;

	
	ret = initSocket((struct sockaddr *)(&m_stAddr), true);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Socket init fail\n");
		closeSocket();
		return ret;
	}

	struct ip_mreq stMreq;
	memset(&stMreq, '\0', sizeof(struct ip_mreq));
	stMreq.imr_interface.s_addr = htonl(INADDR_ANY);
	stMreq.imr_multiaddr.s_addr = inet_addr(pUrl);

	ret = setsockopt(m_iSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &stMreq, sizeof(struct ip_mreq));
	if (ret != AD_SUCCESS) {
		AD_ERROR("Setsockopt fail\n");
		closeSocket();
		return ret;
	}

	return ret;

}

