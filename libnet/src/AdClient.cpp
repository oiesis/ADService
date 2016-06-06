/***************************************
*	Description: server module, detect server and session status
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include "AdClient.h"

AdClient::AdClient(void (*pPreSend)(AdSession *))
{
	m_pPreSend = pPreSend;
}

AdClient::~AdClient()
{
	map<string, AdSession * >::iterator iter;
	for(iter = m_mSession.begin(); iter != m_mSession.end(); iter ++)
	{
		AdNetBaseDeleteSession(iter->second);
	}
	m_mSession.clear();
	m_mServer.clear();
}

int AdClient::AdClientInit(list<string>& ltServer, int msgRead, int msgWrite)
{
	int ret;
	list<string>::iterator iter;
	ret = AdNetBaseInit(msgRead, msgWrite);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("NetBase Init Fail\n");
		return AD_FAILURE;
	}
	
	for(iter = ltServer.begin(); iter != ltServer.end(); iter ++)
	{
		AdClientCreateSession(*iter);
	}

	return ret;
}

int AdClient::AdClientCreateSession(string & sServer)
{
	vector<string> vStr;
	int id;
	int ret;
	AdSession * pSession = NULL;

	AdString::StrSplit(sServer, vStr, ":");
	if(vStr.size() != 2)
	{
		return AD_FAILURE;
	}

	pSession = AdNetBaseNewSession();
	if(pSession == NULL)
	{
		AD_ERROR("Memory not enough\n");
		return AD_FAILURE;
	}

	id = pSession->AdSessionInit(vStr[0], atoi(vStr[1].c_str()));
	if(id <= 0)
	{
		AD_ERROR("Session Init Fail\n");
		AdNetBaseDeleteSession(pSession);
		return AD_FAILURE;
	}

	ret = AdNetBaseAddClient(pSession);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Add Server Fail\n");
		AdNetBaseDeleteSession(pSession);
		return AD_FAILURE;
	}
	if(m_pPreSend)
		m_pPreSend(pSession);
	ctLock.lock();
	m_mSession[sServer] = pSession;
	m_mServer[pSession] = sServer;
	ctLock.unlock();
	return AD_SUCCESS;
}

int AdClient::AdClientDelSession(string & sServer)
{
	map<string, AdSession * >::iterator iter;

	ctLock.lock();
	iter = m_mSession.find(sServer);
	if(iter == m_mSession.end())
	{
		ctLock.unlock();
		return AD_SUCCESS;
	}
	
	AdSession *pSession = iter->second;
	pSession->AdSessionClose();
	m_mServer.erase(pSession);
	m_mSession.erase(iter);
	ctLock.unlock();
	return AD_SUCCESS;
}


int AdClient::AdNetBaseSessionReconnect(AdSession * pSession)
{
	int ret;
	map<AdSession * , string>::iterator iter;
	ctLock.lock();
	iter = m_mServer.find(pSession);
	if(iter != m_mServer.end())
	{
		ret = AdClientCreateSession(iter->second);
		if(ret != AD_SUCCESS)
		{
			AD_ERROR("Try Create Session Fail\n");
			m_mSession.erase(iter->second);
			m_mServer.erase(iter);
		}
	}
	ctLock.unlock();
	return ret;
}
