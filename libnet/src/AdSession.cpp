/***************************************
*	Description: Session module, detect server and session status
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include "AdSession.h"


AdSession::AdSession(AdRingBufferSRMW<AdSession *> * pMsg)
{
	m_iSocket = AD_FAILURE;
	m_pMsg = pMsg;
	m_uiActive = AD_SUCCESS;
	m_iTimeout = 0;
	m_bTimeoutChange = false;
	m_pData = NULL;
	m_iPacketLen = AD_FAILURE;
	m_iReadLen = AD_FAILURE;
}

AdSession::~AdSession()
{
	while(AdSessionClose() != AD_SUCCESS)
		Time::wait(1);
}

int  AdSession::AdSessionClose()
{
	int ret;
	m_ctLock.lock();
	ret = m_ctTcp.Disconnect();
	AdSessionCacheClear(m_ltSendCache);
	AdSessionCacheClear(m_ltReadCache);
	m_ctLock.unlock();

	if(ret == AD_SUCCESS)
	{
		if(m_uiActive != AD_SUCCESS)
			return AD_FAILURE;
	}
	m_iPacketLen = AD_FAILURE;
	m_iReadLen = AD_FAILURE;

	m_iTimeout = 0;
	m_bTimeoutChange = false;
	return ret;
}

void  AdSession::AdSessionCacheClear(list<AdCache *>& ltCache )
{
	list<AdCache *>::iterator  iter;
	for(iter = ltCache.begin(); iter != ltCache.end(); iter ++)
	{
		delete *iter;
	}
	ltCache.clear();
}

void AdSession::GetIp(string & sIp)
{
	m_ctTcp.GetIp(sIp);
}


int AdSession::AdSessionInit(int iSocket)
{
	int sock = m_ctTcp.Accept(iSocket);
	if(sock <= 0)
	{
		return AD_FAILURE;
	}

	int ret = m_ctTcp.NonBlock();
	if(ret == AD_FAILURE)
	{
		AD_ERROR("Tcp set error\n");
		return AD_FAILURE;
	}

	m_iSocket = sock;
	return sock;
}

int AdSession::AdSessionInit(string &ip, int iPort)
{
	int sock = m_ctTcp.Connect(ip.c_str(), iPort);
	if(sock <= 0)
	{
		return AD_FAILURE;
	}

	int ret = m_ctTcp.NonBlock();
	if(ret == AD_FAILURE)
	{
		AD_ERROR("Tcp set error\n");
		return AD_FAILURE;
	}

	m_iSocket = sock;
	return sock;
}

int AdSession::AdSessionRead(list<AdCache *>& ltCache )
{
	AdCache *pCache;
	int iLen;
	
	do
	{
		pCache = new AdCache;
		if(pCache == NULL)
		{
			AD_ERROR("Mem not nough\n");
			return AD_FAILURE;
		}
		m_ctLock.lock();
		iLen = m_ctTcp.Read(pCache->Get(), pCache->Max());
		m_ctLock.unlock();
		if(iLen > 0)
		{
			pCache->Set(iLen);
			ltCache.push_back(pCache);
		}
		else
		{
			delete pCache;
		}
	}while(iLen > 0);
	if(iLen == AD_FAILURE)
	{
		AdSessionCacheClear(ltCache);
		AdSessionClose();
		if(m_pMsg != NULL)
		{
			m_pMsg->push(this);
		}
	}
	return  AD_SUCCESS;
}

AdCache * AdSession::AdSessionCompletePacket(AdPacketInterface * pInterface, AdCache * pCache)
{
	list<AdCache *>::iterator  iter;
	AdCache * pAll;
	int iLen;
	
	//check Packet Len function
	if(pInterface == NULL) 
	{
		return pCache;
	}

	iLen = pInterface->PacketLen(pCache);
	//single complete packet
	if(iLen == pCache->Size())
	{
		if(m_iPacketLen >= 0)
		{
			m_iReadLen = AD_FAILURE;
			m_iPacketLen = AD_FAILURE;
			AdSessionCacheClear(m_ltReadCache);
		}
		return pCache;
	}
	//
	if(iLen == AD_FAILURE)
	{
		//no packet Head, not filter data, return
		if(m_iPacketLen == AD_FAILURE)
		{
			return pCache;
		}
		
		m_iReadLen += pCache->Size();
		m_ltReadCache.push_back(pCache);
		//have read complete packet
		if(m_iPacketLen == m_iReadLen)
		{
			iter = m_ltReadCache.begin();
			m_ltReadCache.pop_front();
			pAll = *iter;
			for(iter = m_ltReadCache.begin(); iter != m_ltReadCache.end(); iter ++)
			{
				pAll->Append((*iter)->Get(), (*iter)->Size());
			}
			m_iReadLen = AD_FAILURE;
			m_iPacketLen = AD_FAILURE;
			AdSessionCacheClear(m_ltReadCache);
			return pAll;
		}
		//not complete, wait next
		else if(m_iPacketLen > m_iReadLen)
		{
			return NULL;
		}

		//error data ignore and clear
		m_iReadLen = AD_FAILURE;
		m_iPacketLen = AD_FAILURE;
		AdSessionCacheClear(m_ltReadCache);
		return NULL;
	}
	
	//find new packet head, clear old uncomplete Data
	if(m_iPacketLen >= 0)
	{
		m_iReadLen = AD_FAILURE;
		m_iPacketLen = AD_FAILURE;
		AdSessionCacheClear(m_ltReadCache);
	}
	m_iPacketLen = iLen;
	m_iReadLen += pCache->Size();
	m_ltReadCache.push_back(pCache);
	return NULL;
}


int AdSession::AdSessionRead(list<AdCache *>& ltCache, AdPacketInterface * pInterface)
{
	AdCache *pCache;
	int iLen;
	
	do
	{
		pCache = new AdCache;
		if(pCache == NULL)
		{
			AD_ERROR("Mem not nough\n");
			return AD_FAILURE;
		}
		m_ctLock.lock();
		iLen = m_ctTcp.Read(pCache->Get(), pCache->Max());
		
		if(iLen > 0)
		{
			pCache->Set(iLen);
			pCache = AdSessionCompletePacket(pInterface, pCache);
			if(pCache != NULL)
				ltCache.push_back(pCache);
		}
		else
		{
			delete pCache;
		}
		m_ctLock.unlock();
	}while(iLen > 0);
	
	if(iLen == AD_FAILURE)
	{
		AdSessionCacheClear(ltCache);
		AdSessionClose();
		if(m_pMsg != NULL)
		{
			m_pMsg->push(this);
		}
	}
	return  AD_SUCCESS;
}

int AdSession::AdSessionWrite(AdCache & ctCache)
{
	int len;
	if(ctCache.Size() <= 0)
		return AD_FAILURE;
	m_ctLock.lock();
	len = m_ctTcp.Write(ctCache.Get(),   ctCache.Size());
	m_ctLock.unlock();

	if(len == AD_FAILURE)
	{
		AdSessionClose();
		if(m_pMsg != NULL)
		{
			m_pMsg->push(this);
		}
	}

	if(len == AD_SUCCESS)
	{
		AdCache *pNetCache;
		pNetCache = new AdCache(ctCache);
		if(pNetCache == NULL)
		{
			AD_ERROR("Mem not nough\n");
			return AD_FAILURE;
		}
		m_ctLock.lock();
		m_ltSendCache.push_back(pNetCache);
		m_ctLock.unlock();
		if(m_pMsg)
		{
			m_pMsg->push(this);
		}
	}
	return AD_SUCCESS;
}

int AdSession::AdSessionWrite(const char * pbuff, int iLen)
{
	int len;
	if(iLen <= 0)
		return AD_FAILURE;
	m_ctLock.lock();
	len = m_ctTcp.Write(pbuff,   iLen);
	m_ctLock.unlock();

	if(len == AD_FAILURE)
	{
		AdSessionClose();
		if(m_pMsg != NULL)
		{
			m_pMsg->push(this);
		}
	}

	if(len == AD_SUCCESS)
	{
		AdCache *pCache;
		pCache = new AdCache(pbuff, iLen);
		if(pCache == NULL)
		{
			AD_ERROR("Mem not nough\n");
			return AD_FAILURE;
		}
		m_ctLock.lock();
		m_ltSendCache.push_back(pCache);
		m_ctLock.unlock();
		if(m_pMsg)
		{
			m_pMsg->push(this);
		}
	}
	return AD_SUCCESS;
}


int AdSession::AdSessionWrite()
{
	AdCache *pCache;
	int len;
	int iSize;
	m_ctLock.lock();
	while(!m_ltSendCache.empty())
	{
		pCache = m_ltSendCache.front();
		m_ltSendCache.pop_front();
		len = m_ctTcp.Write(pCache->Get(),   pCache->Size());
		if(len == AD_SUCCESS)
		{
			m_ltSendCache.push_front(pCache);
			break;
		}
		else if(len == AD_FAILURE)
		{
			break;
		}
		delete pCache;
	}
	iSize = m_ltSendCache.size();
	m_ctLock.unlock();
	
	if(len == AD_FAILURE)
	{
		AdSessionClose();
		if(m_pMsg != NULL)
		{
			m_pMsg->push(this);
		}
	}

	return iSize;
}

bool AdSession::AdSessionEmpty()
{
	return m_ltSendCache.empty();
}

bool AdSession::AdSessionOk()
{
	if(m_iSocket == AD_FAILURE)
		return false;
	
	return (m_iSocket == m_ctTcp.GetSocket());
}

int AdSession::AdSessionGetSocket()
{
	return m_iSocket;
}

int AdSession::AdSessionActive()
{
	m_ctLock.lock();
	m_uiActive ++;
	m_ctLock.unlock();
	return m_uiActive;
}

int AdSession::AdSessionSetPrivate(void * pData)
{
	m_ctLock.lock();
	m_pData = pData;
	m_ctLock.unlock();
	return AD_SUCCESS;
}

void * AdSession::AdSessionGetPrivate(bool bClear)
{
	void * pData; 
	m_ctLock.lock();
	pData = m_pData;
	if(bClear)
		m_pData = NULL;
	m_ctLock.unlock();
	return pData;
}



int AdSession::AdSessionIdle()
{
	m_ctLock.lock();
	if(m_uiActive > 0)
		m_uiActive --;
	m_ctLock.unlock();
	return m_uiActive;
}

int AdSession::AdSessionSetTimeout(int iMs)
{
	if(m_iTimeout != iMs)
	{
		m_ctLock.lock();
		m_iTimeout = iMs;
		m_bTimeoutChange = true;
		m_ctLock.unlock();
		//update msg for netbase
		if(m_pMsg != NULL)
		{
			m_pMsg->push(this);
		}
	}
	return AD_SUCCESS;
}

int AdSession::AdSessionGetTimeout()
{
	if(m_bTimeoutChange)
	{
		m_bTimeoutChange = false;
		return m_iTimeout;
	}
	return AD_FAILURE;
}

