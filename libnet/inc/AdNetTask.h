/***************************************
*	Description: Ad Server Task module
*	Author: wilson
*	Date: 2015/09/15
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_SERVER_TASK_H__
#define __AD_SERVER_TASK_H__

#include <pthread.h>
#include <stdlib.h>
#include "AdGlobal.h"
#include "AdSession.h"
#include "AdNetBase.h"
#include "AdTask.h"

using namespace std;

class AdNetTaskInterface:public AdPacketInterface
{
public:
	AdNetTaskInterface()
	{
		m_sName = "Test";
	};
	virtual ~AdNetTaskInterface()
	{
	};

	virtual  int Init(AdTask & ctTask)
	{
		return AD_SUCCESS;
	};
	
	virtual  int DeInit()
	{
		return AD_SUCCESS;
	};
	
	virtual  int Run(AdCache * pCache, AdSession * pSession, AdTask & ctTask)
	{
		return AD_SUCCESS;
	};
	
	virtual  int PacketLen(AdCache* pCache)
	{
		return pCache->Size();
	};

	void SetName(const string & sName)
	{
		m_sName = sName;
	};

	string & GetName()
	{
		return m_sName;
	}
private:
	string m_sName;
};

class AdServerTask:public AdTask
{
public:
	AdServerTask(AdNetBase * pBase, AdNetTaskInterface * pInterface)
	{
		if((pBase == NULL) || (pInterface == NULL))
		{
			AD_ERROR("NetTask Para error\n");
		}
		m_pNetBase = pBase;
		m_pInterface = pInterface;
		m_sDescription = pInterface->GetName();
		m_sDescription += "- AD Server Thread";
	};

	virtual ~AdServerTask()
	{
	};

	virtual void TaskLoop()
	{
		int ret;
		int iError = 0;
		while(m_bTaskRun)
		{
			SetTaskWaitting(true);
			ret = m_pNetBase->AdNetBaseRun();
			if(ret != AD_SUCCESS)
			{
				iError ++;
				if(iError % 100 == 99)
					AD_ERROR("Server Task Error  %d\n", iError);
			}
			m_uiLoop ++;
			Time::waitNanoSec(2000);
		}
		m_bTaskRun = false;
		return ;
	}
private:
	AdNetBase *     m_pNetBase;
	AdNetTaskInterface  * m_pInterface;
};

class AdWorkerTask:public AdTask
{
public:
	AdWorkerTask(AdNetBase * pBase, AdNetTaskInterface * pInterface)
	{
		if((pBase == NULL) || (pInterface == NULL))
		{
			AD_ERROR("NetTask Para error\n");
		}
		m_pNetBase = pBase;
		m_pInterface = pInterface;
		m_sDescription = pInterface->GetName();
		m_sDescription += "- AD Worker Thread";
	};

	virtual ~AdWorkerTask()
	{
	};

	virtual void TaskLoop()
	{
		int ret;
		list<AdCache *> ltRead;
		list<AdCache *>::iterator iter;
		
		AdRingBufferSRSW<AdSession *> *pMsg = m_pNetBase->AdNetBaseGetMsg(m_uiTaskId & AD_TASK_MASK_ID, AD_NET_MSG_READ);
		AdSession *pSession;
	
		//have pre init function
		m_pInterface->Init(*static_cast<AdTask *>(this));
	
		//main loop
		while(m_bTaskRun)
		{
			SetTaskWaitting(true);
			//get Session from msg
			pSession = static_cast<AdSession *>(pMsg->pop(1000));
			if(pSession == NULL)
			{
				AD_ERROR("Get Session Error\n");
				continue;
			}
			SetTaskWaitting(false);
			//clear read cache
			pSession->AdSessionCacheClear(ltRead);
			//read Data, return data list
			ret = pSession->AdSessionRead(ltRead, static_cast<AdPacketInterface * >(m_pInterface));
			if(ret != AD_SUCCESS)
			{
				AD_ERROR("Session Read Error\n");
				pSession->AdSessionIdle();
				continue;
			}
	
			//call packet handle
			for(iter = ltRead.begin(); iter != ltRead.end(); iter ++)
			{
				if((*iter)->Size() <= 0)
					continue;
				m_pInterface->Run(*iter,pSession, *static_cast<AdTask *> (this));
			}
			if(ltRead.empty())
			{
				m_pInterface->Run(NULL, pSession,  *static_cast<AdTask *> (this));
			}
			//sub Session active counter
			pSession->AdSessionIdle();
			m_uiLoop++;
		}
		m_bTaskRun = false;
		m_pInterface->DeInit();
		return ;
	}
public:
	AdNetBase* getNetBase(){
		return m_pNetBase;
	}
private:
	AdNetBase *     m_pNetBase;
	AdNetTaskInterface  * m_pInterface;
};


class AdWriteTask:public AdTask
{
public:
	AdWriteTask(AdNetBase * pBase, AdNetTaskInterface * pInterface)
	{
		if((pBase == NULL) || (pInterface == NULL))
		{
			AD_ERROR("NetTask Para error\n");
		}
		m_pNetBase = pBase;
		m_pInterface = pInterface;
		m_sDescription = pInterface->GetName();
		m_sDescription += "- AD Write Thread";
	};

	virtual ~AdWriteTask()
	{
	};

	virtual void TaskLoop()
	{
		AdRingBufferSRSW<AdSession *> *pMsg = m_pNetBase->AdNetBaseGetMsg(m_uiTaskId & AD_TASK_MASK_ID, AD_NET_MSG_WRITE);
		AdSession *pSession;

		while(m_bTaskRun)
		{
			SetTaskWaitting(true);
			pSession = static_cast<AdSession *>(pMsg->pop());
			if(pSession == NULL)
			{
				AD_ERROR("Get Session Error\n");
				continue;
			}
			SetTaskWaitting(false);
			pSession->AdSessionWrite();
			m_uiLoop ++;
		}
		m_bTaskRun = false;
		return ;
	}
private:
	AdNetBase *     m_pNetBase;
	AdNetTaskInterface  * m_pInterface;
};


template< typename Para1, typename Para2>
int  AdNetTaskCreate(Para1  p1, Para2  p2,  int iRead, int iWrite)
{
	AdServerTask * pServer; 

	pServer = AdTaskCreate<AdServerTask, Para1, Para2>( 1 | AD_TASK_SERVER, p1, p2);
	if(pServer == NULL)
	{
		AD_ERROR("AdServerTask Create Error\n");
		return AD_FAILURE;
	}

	while(iRead > 0)
	{
		AdWorkerTask * pWork= AdTaskCreate<AdWorkerTask, Para1, Para2>( iRead | AD_TASK_WORKER, p1, p2);
		if(pWork == NULL)
		{
			AD_ERROR("AdWorkerTask Create Error\n");
			return AD_FAILURE;
		}
		iRead --;
	}

	while(iWrite > 0)
	{
		AdWriteTask * pWrite = AdTaskCreate<AdWriteTask, Para1, Para2>(iWrite | AD_TASK_WRITE, p1, p2);
		if(pWrite == NULL)
		{
			AD_ERROR("AdWorkerTask Create Error\n");
			return AD_FAILURE;
		}
		iWrite --;
	}
	
	return AD_SUCCESS;
}

#endif

