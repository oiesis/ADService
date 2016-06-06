/***************************************
*	Description: tcp module
*	Author: wilson
*	Date: 2015/08/14
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include <error.h>
#include "AdTcp.h"

AdTcp::AdTcp()
{
	m_iSocket = AD_FAILURE;
}

AdTcp::~AdTcp()
{
	Disconnect();
}

int  AdTcp::Disconnect()
{
	int ret;
	if(m_iSocket != AD_FAILURE)
	{
		ret = close(m_iSocket);
		if(ret == AD_SUCCESS)
		{
			m_iSocket = AD_FAILURE;
			m_sIp.clear();
		}
		return ret;
	}
	return AD_SUCCESS;
}

int AdTcp::GetSocket()
{
	return m_iSocket;
}


int AdTcp::Accept(int iServerSocket)
{
	UINT addr_len=sizeof(struct sockaddr_in);
	m_iSocket = accept(iServerSocket, (struct sockaddr *)(&m_stAddr), &addr_len);
	m_sIp = inet_ntoa(*(struct in_addr*)&m_stAddr.sin_addr.s_addr);
	return m_iSocket;
}

void AdTcp::GetIp(std::string & sIp)
{
	sIp = m_sIp;
}

int AdTcp::NonBlock()
{
	if (AD_FAILURE == fcntl(m_iSocket, F_SETFL, fcntl(m_iSocket, F_GETFL, 0) | O_NONBLOCK)) 
	{
		AD_ERROR("Fcntl error\n");
		return AD_FAILURE;
	}

	return AD_SUCCESS;
}


int AdTcp::Connect(const char * ip, int iPort)
{
	int ret;
	
	m_iSocket=socket(AF_INET,SOCK_STREAM,0);
	if(m_iSocket == AD_FAILURE)
	{
		AD_ERROR("Socket init Error\n");
		 return AD_FAILURE;
	}

	bzero(&m_stAddr,sizeof(struct sockaddr_in));
	m_stAddr.sin_family=AF_INET;
	m_stAddr.sin_port=htons(iPort);
	m_stAddr.sin_addr.s_addr = inet_addr(ip);
	m_sIp = ip;

	ret = connect(m_iSocket,(struct sockaddr *)(&m_stAddr),sizeof(struct sockaddr));
	if(ret != AD_SUCCESS)
	{
		 AD_ERROR("Socket connect Fail\n");
		 Disconnect();
		 return AD_FAILURE;
	}

	return m_iSocket;
}

int AdTcp::Write(const char* pPacket, int iLength)
{
	int ret;
	if(m_iSocket > 0)
	{
		ret = write(m_iSocket, pPacket, iLength);
		if(ret == 0)
			return AD_FAILURE;
		else if(ret == AD_FAILURE)
		{
			if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
				return AD_SUCCESS;
		}
		return ret;
	}
	else
		return AD_FAILURE;
}

int AdTcp::Read(char * pPacket, int iLength,  int wait)
{
   	fd_set inputs;   
  	struct timeval timeout;   
	int result;
	int ret;

	if(m_iSocket <= 0)
		return AD_FAILURE;
	
	if(wait > 0)
	{	 
		FD_ZERO(&inputs);  
		FD_SET(m_iSocket,&inputs); 
		timeout.tv_sec = wait/1000000;   
		timeout.tv_usec = wait%1000000;   
		result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, &timeout);   
		if(result <= 0)
		{
			return AD_SUCCESS;
		}
		if(FD_ISSET(m_iSocket,&inputs))   
		{   
			ret = read(m_iSocket, pPacket, iLength);
			if(ret == 0)
			{
				return AD_FAILURE;
			}
			else if(ret == AD_FAILURE)
			{
				if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
					return AD_SUCCESS;
			}
			return ret;
		}   
		return AD_SUCCESS;
	}
	else
	{
		ret = read(m_iSocket, pPacket, iLength);
		if(ret == 0)
			return AD_FAILURE;
		else if(ret == AD_FAILURE)
		{
			if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
				return AD_SUCCESS;
		}
		return ret;
	}
}

