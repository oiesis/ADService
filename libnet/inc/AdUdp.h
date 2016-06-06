/***************************************
*	Description: base class for net.
*	Author: wilson
*	Date: 2015/07/20
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef _AD_UDP_H_
#define	_AD_UDP_H_
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "AdGlobal.h"

class AdUdp 
{
public:
	AdUdp();
	virtual ~AdUdp();

	int UdpSend(char* pPacket, int iLength);
	int UdpRev(char* pPacket, int iLength, int wait);
	int UdpInit(void* pAddr);
	int UdpInit(char * sUrl, int iPort);
	int UdpMultiInit(char * sUrl, int iPort);

	AdUdp& operator=(const AdUdp &a) 
	{
		this->m_iSocket = a.m_iSocket;
		return *this;
	}

protected:
	int initSocket(struct sockaddr* pAddr, bool bReUse, bool bBind = true);
	void closeSocket();
private:
	struct sockaddr_in m_stAddr;
	int m_iSocket;
	bool  m_iSocketInit;
};


inline int AdUdp::UdpSend(char* pPacket, int iLength)
{
	return sendto(m_iSocket, pPacket, iLength, 0, (struct sockaddr*)&m_stAddr, sizeof(struct sockaddr_in));
}

inline int AdUdp::UdpRev(char * pPacket, int iLength,  int wait = 0)
{
   	 fd_set inputs;   
  	struct timeval timeout;   
	int result;
	
	int iUdp = sizeof(struct sockaddr_in);
	
	if(wait > 0)
	{	 
    		FD_ZERO(&inputs);  
     		FD_SET(m_iSocket,&inputs); 
       		timeout.tv_sec = wait/1000000;   
      		timeout.tv_usec = wait%1000000;   
      		result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, &timeout);   
       		if(result <= 0)
       		{
       			return AD_FAILURE;
       		}
          	if(FD_ISSET(m_iSocket,&inputs))   
          	{   
           		return recvfrom(m_iSocket, pPacket, iLength, 0, (struct sockaddr*)&m_stAddr, (socklen_t*)&iUdp);
                }   
		return AD_FAILURE;
	}
	else
		return recvfrom(m_iSocket, pPacket, iLength, 0, (struct sockaddr*)&m_stAddr, (socklen_t*)&iUdp);
}
#endif
