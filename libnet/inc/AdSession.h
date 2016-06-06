/***************************************
*	Description: Session module, detect server and session status
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#ifndef   __AD_SESSION_H__
#define __AD_SESSION_H__

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include "AdTcp.h"
#include "AdGlobal.h"
#include "AdBuff.h"
#include "AdLock.h"
#include "AdUtil.h"
#include "AdTime.h"


using namespace std;

class AdPacketInterface
{
public:
	AdPacketInterface(){};
	virtual ~AdPacketInterface(){};
	virtual int PacketLen(AdCache* pCache)
	{
		return pCache->Size();
	};
};

class AdSession
{
public:
	AdSession(AdRingBufferSRMW<AdSession *> * pMsg = NULL);
	~AdSession();
	//init Session,  server or client
	int AdSessionInit(int iSocket);
	int AdSessionInit(string & ip, int iPort);
	//read write data
	int AdSessionRead(list<AdCache *>& ltCache );
	//read complete packet
	int AdSessionRead(list<AdCache *>& ltCache, AdPacketInterface * pInterface);
	int AdSessionWrite(AdCache& ctCache);
	int AdSessionWrite(const char * pbuff, int iLen);
	//delay write, call by netbase
	int AdSessionWrite();
	//check writebuff empty 
	bool AdSessionEmpty();
	//get session sock
	int AdSessionGetSocket();
	//clear net cache
	void  AdSessionCacheClear(list<AdCache *>& ltCache);
	//close session
	int  AdSessionClose();
	//check session ok, or close
	bool AdSessionOk();
	//check session activty or idle
	int AdSessionActive();
	int AdSessionIdle();
	void GetIp(string & sIp);

	int AdSessionSetPrivate(void * pData);
	void * AdSessionGetPrivate(bool bClear = false);
	
	//set/get session read timeout
	int AdSessionSetTimeout(int iMs);
	int AdSessionGetTimeout();
	
private:
	AdCache * AdSessionCompletePacket(AdPacketInterface * pInterface, AdCache * pCache);
	AdTcp  m_ctTcp;
	list<AdCache *> m_ltSendCache;
	LockMutex m_ctLock;
	int m_iSocket;
	AdRingBufferSRMW<AdSession *>  *m_pMsg;
	UINT  m_uiActive;
	int  m_iTimeout;
	bool  m_bTimeoutChange;
	void * m_pData;

	list<AdCache *> m_ltReadCache;
	int m_iPacketLen;
	int m_iReadLen;
};
#endif

