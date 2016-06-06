/***************************************
*	Description: Net base module, detect net status
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_NET_BASE_H__
#define __AD_NET_BASE_H__

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include "AdTcp.h"
#include "AdTcpServer.h"
#include "AdSession.h"
#include "AdEvent.h"
#include "AdGlobal.h"
#include "AdBuff.h"
#include "AdTime.h"

#define AD_NET_MSG_READ           1
#define AD_NET_MSG_WRITE        2

#define AD_TASK_SESSION_MAX                        (1024 * 32)

class AdNetBase:public AdEvent
{
public:
	AdNetBase();
	~AdNetBase();
	int AdNetBaseInit(int msgRead, int msgWrite);
	AdRingBufferSRSW<AdSession *> * AdNetBaseGetMsg(int id,  UINT uiType);
	int AdNetBaseRun();

protected:
	//run behind session close
	virtual int AdNetBaseSessionReconnect(AdSession * pSession);
	//must call before system run
	int AdNetBaseAddServer(int iSocket);
	//call any time.
	int AdNetBaseAddClient(AdSession * pSession);
	//get new session
	AdSession * AdNetBaseNewSession();
	//delete session
	void AdNetBaseDeleteSession(AdSession * pSession);

private:
	int AdNetBaseAddSession(AdSession * pSession);
	int AdNetBaseCloseSession(AdSession * pSession);
	void AdNetBasePushMsg(AdSession * pSession, UINT uiType);
	
	virtual int AdEventHandleMsg(int iSocket, int iMsgType);
	int AdNetBaseLoop();

	//session: detect read msg
	map<int, AdSession *> m_mSessionRead;                     //all session, all detect read msg
	//session: detect Write msg
	map<int, AdSession *> m_mSessionWrite;     
	//session: close wait delete
	map<AdSession *, AdSession *> m_mSessionClose;     
	//session:close  wait timeout
	multimap<Time_t, AdSession *> m_mSessionCloseTime;     
	//session:  close wait reuse
	AdRingBufferSRSW<AdSession *>   m_ctSessionFree;
	//session: prepare detect event
	AdRingBufferSRMW<AdSession *>   m_ctSessionNew;

	//thread msg, from server to work thread
	vector<AdRingBufferSRSW<AdSession *> *> m_vMsgRead;
	UINT m_uiRead;
	//thread msg, from work thread to server
	vector<AdRingBufferSRSW<AdSession *> *> m_vMsgWrite;
	UINT m_uiWrite;

	//session control msg, from work thread to server
	AdRingBufferSRMW<AdSession *> m_ctMsgControl;
	Time m_ctTime;

	int m_iServer;

	//status count;
	UINT  m_uiAllSession;
	UINT  m_uiDelSession;
	UINT  m_uiCloseSession;
};

#endif
