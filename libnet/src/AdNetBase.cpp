/***************************************
*	Description: net base module, detect net status, send msg
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include "AdNetBase.h"

#define AD_PRINT_INTERNAL    10000

AdNetBase::AdNetBase():m_ctSessionFree(AD_TASK_SESSION_MAX),
 	m_ctSessionNew(AD_TASK_SESSION_MAX),m_ctMsgControl(AD_TASK_SESSION_MAX)
{
	m_uiRead = 0;
	m_uiWrite = 0;
	m_iServer = AD_FAILURE;

	m_uiAllSession = 0;
	m_uiCloseSession = 0;
	m_uiDelSession = 0;
}

AdNetBase::~AdNetBase()
{
	map<int, AdSession *>::iterator mIter;
	map<AdSession *, AdSession *>::iterator mIter2;
	vector<AdRingBufferSRSW<AdSession *> *>::iterator vIter;

	//clear write msg session
	m_mSessionWrite.clear();

	//clear read msg session
	mIter = m_mSessionRead.begin(); 
	while(mIter != m_mSessionRead.end())
	{
		AdEventDel(mIter->first);
		delete  mIter->second;
		mIter ++;
	}
	m_mSessionRead.clear();

	//clear close session 
	mIter2 = m_mSessionClose.begin(); 
	while(mIter2 != m_mSessionClose.end())
	{
		delete  mIter2->second;
	}
	m_mSessionClose.clear();
	m_mSessionCloseTime.clear();
	
	//clear msg read
	for(vIter = m_vMsgRead.begin();  vIter != m_vMsgRead.end(); vIter ++)
	{
		delete *vIter;
	}
	m_vMsgRead.clear();
	m_vMsgWrite.clear();

	AdSession * pSession; 
	while(m_ctSessionFree.tryPop(pSession))
	{
		delete pSession;
	}
}

AdRingBufferSRSW<AdSession *> * AdNetBase::AdNetBaseGetMsg(int id,  UINT uiType)
{
	AdRingBufferSRSW<AdSession *>  * pMsg;
	switch(uiType)
	{
		case AD_NET_MSG_READ:
			if(m_vMsgRead.empty())
			{
				pMsg = NULL;
			}
			else
			{
				pMsg = m_vMsgRead[id % m_vMsgRead.size()];
			}
			break;
		case AD_NET_MSG_WRITE:
			if(m_vMsgWrite.empty())
			{
				pMsg = NULL;
			}
			else
			{
				pMsg = m_vMsgWrite[id % m_vMsgWrite.size()];
			};
			break;	
		default:
			break;
	}
	return pMsg;
}

int AdNetBase::AdNetBaseInit( int msgRead, int msgWrite)
{
	int i;
	AdRingBufferSRSW<AdSession *> * pMsg;
	
	if(msgRead <= 0)
		msgRead = 1;
	for(i = 0;  i < msgRead; i ++)
	{
		pMsg = new AdRingBufferSRSW<AdSession *>(AD_TASK_SESSION_MAX);
		if(pMsg == NULL)
		{
			AD_ERROR("mem not enough\n");
			return AD_FAILURE;
		}
		m_vMsgRead.push_back(pMsg);
	}

	if(msgWrite <= 0)
		msgWrite = 1;
	for(i = 0;  i < msgWrite; i ++)
	{
		pMsg = new AdRingBufferSRSW<AdSession *>(AD_TASK_SESSION_MAX);
		if(pMsg == NULL)
		{
			AD_ERROR("mem not enough\n");
			return AD_FAILURE;
		}
		m_vMsgWrite.push_back(pMsg);
	}
	return AD_SUCCESS;
}

void AdNetBase::AdNetBasePushMsg(AdSession * pSession, UINT uiType)
{
	switch(uiType)
	{
		case AD_NET_MSG_READ:
			pSession->AdSessionActive();
			while(m_vMsgRead[m_uiRead]->tryPush(pSession) != true)
			{
				m_uiRead ++;
				if(m_uiRead >= m_vMsgRead.size())
					m_uiRead = 0;
			}
			m_uiRead ++;
			if(m_uiRead >= m_vMsgRead.size())
					m_uiRead = 0;
			break;
		case AD_NET_MSG_WRITE:
			pSession->AdSessionActive();
			while(m_vMsgWrite[m_uiRead]->tryPush(pSession) != true)
			{
				m_uiWrite ++;
				if(m_uiWrite >= m_vMsgWrite.size())
					m_uiWrite = 0;
			}
			break;	
		default:
			break;
	}
}

AdSession * AdNetBase::AdNetBaseNewSession()
{
	AdSession * pSession; 
	if(!m_ctSessionFree.tryPop(pSession))
	{
		pSession = new AdSession(&m_ctMsgControl);
		if(pSession == NULL)
		{
			AD_ERROR("Mem not enough\n");
			return NULL;
		}
	}
	return pSession;
}

void AdNetBase::AdNetBaseDeleteSession(AdSession * pSession)
{
	if(!m_ctSessionFree.tryPush(pSession))
	{
		delete pSession;
	}
	m_uiDelSession ++;
	if(m_uiDelSession % AD_PRINT_INTERNAL == (AD_PRINT_INTERNAL  - 1))
	{
		AD_INFO("All:%d  Close:%d Del:%d -- read:%d  write:%d  close:%d-%d free:%d\n", 
			m_uiAllSession, m_uiCloseSession, m_uiDelSession,
			(int)m_mSessionRead.size(),(int)m_mSessionWrite.size(), 
			(int)m_mSessionClose.size(),(int)m_mSessionCloseTime.size(), m_ctSessionFree.size());	
	}
}

int AdNetBase::AdNetBaseSessionReconnect(AdSession * pSession)
{
	return AD_SUCCESS;
}


int AdNetBase::AdEventHandleMsg(int iSocket, int iMsgType)
{
	AdSession * pSession;
	map<int, AdSession * >::iterator iter;
	int ret;
	int sock;

	//connect new session
	if((iMsgType == AD_EVENT_READ) && (iSocket == m_iServer))
	{
		do
		{
			pSession = AdNetBaseNewSession();
			if(pSession == NULL)
			{
				AD_ERROR("Mem not enough\n");
				return AD_FAILURE;
			}
			sock = pSession->AdSessionInit(iSocket);
			if(sock != AD_FAILURE)
			{
				ret = AdNetBaseAddSession(pSession);
				if(ret != AD_SUCCESS)
				{
					AD_ERROR("Session Add Fail %d\n", pSession->AdSessionGetSocket());
					AdNetBaseDeleteSession(pSession);
					return AD_FAILURE;
				}
			}
			else
			{
				//no active
				m_uiDelSession --;
				AdNetBaseDeleteSession(pSession);
			}
		}while(sock != AD_FAILURE);
		
		return AD_SUCCESS;
	}

	//check socket ok
	if(m_mSessionRead.find(iSocket) == m_mSessionRead.end())
	{
		AD_ERROR("Socket Error  %d\n", iSocket);
		ret = AdEventDel(iSocket);
		if(ret != AD_SUCCESS)
		{
			AD_ERROR("Event del Fail\n");
			return AD_FAILURE;
		}
		return AD_FAILURE;
	}

	pSession = m_mSessionRead[iSocket];
	switch(iMsgType)
	{
		case AD_EVENT_READ:
		case AD_EVENT_TIMEOUT:
			//put pSession for work thread
			AdNetBasePushMsg(pSession, AD_EVENT_READ);
			break;
		case AD_EVENT_WRITE:
			//put pSession for work thread
			AdNetBasePushMsg(pSession, iMsgType);
			//write enable,  delete epoll
			ret = AdEventModify(iSocket, AD_EVENT_READ);
			if(ret != AD_SUCCESS)
			{
				AD_ERROR("Event Add Fail\n");
			}
			break;
		case AD_EVENT_ERROR:
			AdNetBaseCloseSession(pSession);
			//put pSession for work thread
			AdNetBasePushMsg(pSession, AD_EVENT_READ);
			AD_ERROR("Error msg: delete %d\n",iSocket);
			break;
		default:
			AdNetBaseCloseSession(pSession);
			AD_ERROR("Handle msg Error  %d\n", iMsgType);
			break;
	}
	AdNetBaseLoop();
	return AD_SUCCESS;
}

int AdNetBase::AdNetBaseAddServer(int iSocket)
{
	map<int, AdSession * >::iterator iter;
	int ret;
	iter = m_mSessionRead.find(iSocket);
	if(iter != m_mSessionRead.end())
	{
		AD_ERROR("Detect same socket\n");
		return AD_FAILURE;
	}

	ret = AdEventAdd(iSocket, AD_EVENT_READ);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Event Add Fail\n");
		return AD_FAILURE;
	}
	m_iServer = iSocket;
	return AD_SUCCESS;
}

int AdNetBase::AdNetBaseAddClient(AdSession * pSession)
{
	m_ctSessionNew.push(pSession);
	return AD_SUCCESS;
}


int AdNetBase::AdNetBaseAddSession(AdSession * pSession)
{
	map<int, AdSession * >::iterator iter;
	int ret;
	int sock = pSession->AdSessionGetSocket();
	iter = m_mSessionRead.find(sock);
	if(iter != m_mSessionRead.end())
	{
		if(pSession != iter->second)
		{
			//Detect same socket delete old;
			AdNetBaseCloseSession(iter->second);
		}
	}

	ret = AdEventAdd(sock, AD_EVENT_READ);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Event Add Fail\n");
		return AD_FAILURE;
	}
	m_mSessionRead[sock] = pSession;

	m_uiAllSession ++;
	return AD_SUCCESS;
}

int AdNetBase::AdNetBaseCloseSession(AdSession * pSession)
{
	pair<Time_t,AdSession * > p1(m_ctTime.getSecond(),pSession);
	map<int, AdSession * >::iterator iter;
	int iSocket = pSession->AdSessionGetSocket();

	if(m_mSessionClose.find(pSession) != m_mSessionClose.end())
		return AD_SUCCESS;

	iter = m_mSessionRead.find(iSocket);
	if(iter != m_mSessionRead.end())
	{
		if(iter->second == pSession)
		{
			AdEventDel(iSocket);
			m_mSessionWrite.erase(iSocket);
			m_mSessionRead.erase(iSocket);
			AdNetBaseSessionReconnect(pSession);
		}
	}
	pSession->AdSessionClose();
	m_mSessionClose[pSession] =pSession;
	m_mSessionCloseTime.insert(p1);

	m_uiCloseSession ++;
	return AD_SUCCESS;
}

int AdNetBase::AdNetBaseRun()
{
	AdNetBaseLoop();
	AdEventWait(1);
	return AD_SUCCESS;
}

int AdNetBase::AdNetBaseLoop()
{
	int ret;
	map<int, AdSession *>::iterator iter;
	AdSession * pSession;

	//get system Now
	m_ctTime.update();

	//update new session for system, client mode only
	while(m_ctSessionNew.tryPop(pSession))
	{
		ret = AdNetBaseAddSession(pSession);
		if(ret != AD_SUCCESS)
		{
			AD_ERROR("Session Add Fail %d\n", pSession->AdSessionGetSocket());
			AdNetBaseDeleteSession(pSession);
			AdNetBaseSessionReconnect(pSession);
		}
	}
	
	//get write msg	
	while(m_ctMsgControl.tryPop(pSession))
	{
		if(!pSession->AdSessionOk())
		{
			AdNetBaseCloseSession(pSession);
		}
		else if(m_mSessionRead.find(pSession->AdSessionGetSocket())  != m_mSessionRead.end())
		{
			m_mSessionWrite[pSession->AdSessionGetSocket()] = pSession;
		}

		ret = pSession->AdSessionGetTimeout();
		if(ret >= 0)
		{
			ret = AdEventModify(pSession->AdSessionGetSocket(), 
					AdEventState(pSession->AdSessionGetSocket()), ret);
			if(ret != AD_SUCCESS)
			{
				AD_ERROR("Event modify Fail\n");
				return AD_FAILURE;
			}
		}
	}

	//active write 
	iter = m_mSessionWrite.begin(); 
	while(iter != m_mSessionWrite.end())
	{
		pSession = iter->second;
		if(!pSession->AdSessionEmpty())
		{
			if(AdEventState(iter->first) != (AD_EVENT_READ | AD_EVENT_WRITE))
			{
				ret = AdEventModify(iter->first, AD_EVENT_READ | AD_EVENT_WRITE);
				if(ret != AD_SUCCESS)
				{
					AD_ERROR("Event modify Fail\n");
					return AD_FAILURE;
				}
			}
		}
		else
		{
			m_mSessionWrite.erase(iter ++);
			continue;
		}
		iter ++;
	}

	multimap<Time_t, AdSession * >::iterator iter2;
	//detect close socket list, delay delete
	iter2 = m_mSessionCloseTime.begin(); 
	while(iter2 != m_mSessionCloseTime.end())
	{
		if(m_ctTime.getSecond() > (iter2->first + 1))
		{
			ret = iter2->second->AdSessionClose();
			if(ret == AD_SUCCESS)
			{
				AdNetBaseDeleteSession(iter2->second);
				m_mSessionClose.erase(iter2->second);	
				m_mSessionCloseTime.erase(iter2 ++);
			}
			else
			{
				iter2 ++;
			}
			continue;
		}

		break;
	}

	return AD_SUCCESS;
}

