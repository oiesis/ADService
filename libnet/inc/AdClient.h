/***************************************
*	Description: client module, init client
*	Author: wilson
*	Date: 2015/09/17
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_CLIENT_H__
#define __AD_CLIENT_H__

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include "AdTcp.h"
#include "AdSession.h"
#include "AdGlobal.h"
#include "AdNetBase.h"
#include "AdUtil.h"

class AdClient:public AdNetBase
{
public:
	AdClient(void (*pPreSend)(AdSession *));
	~AdClient();
	int AdClientInit(list<string>& ltServer, int msgRead, int msgWrite);

	int AdClientDelSession(string & sServer);
	int AdClientCreateSession(string & sServer);
	
private:
	virtual int AdNetBaseSessionReconnect(AdSession * pSession);

	map<string, AdSession * > m_mSession;
	map< AdSession *,string> m_mServer;
	void (*m_pPreSend)(AdSession *);
	LockMutex ctLock;
};

#endif
