/***************************************
*	Description: mult mode, include server client
*	Author: wilson
*	Date: 20150305
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_MULT_H__
#define __AD_MULT_H__

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include "AdTcp.h"
#include "AdSession.h"
#include "AdGlobal.h"
#include "AdNetBase.h"
#include "AdUtil.h"
#include "AdTcpServer.h"

class AdMult:public AdNetBase
{
public:
	AdMult(void (*pPreSend)(AdSession *));
	~AdMult();
	int Init( int msgRead, int msgWrite, int iPort = AD_FAILURE);

	int DelSession(string & sServer);
	int DelSession(AdSession * pSession);
	AdSession * GetSession(string & sServer);
	AdSession * CreateSession(string & sServer);
	
private:
	virtual int AdNetBaseSessionReconnect(AdSession * pSession);

	map<string, AdSession * > m_mSession;
	map< AdSession *,string> m_mServer;
	void (*m_pPreSend)(AdSession *);
	LockMutex ctLock;

	AdTcpServer m_cTcpServer;
};

#endif

