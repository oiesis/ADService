/***************************************
*	Description: mult mode, include server client
*	Author: wilson
*	Date: 20150305
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#include "AdMult.h"

AdMult::AdMult(void (*pPreSend)(AdSession *))
{
	m_pPreSend = pPreSend;
}

AdMult::~AdMult()
{
	map<string, AdSession * >::iterator iter;
	for(iter = m_mSession.begin(); iter != m_mSession.end(); iter ++)
	{
		AdNetBaseDeleteSession(iter->second);
	}
	m_mSession.clear();
	m_mServer.clear();
}

int AdMult::Init(int msgRead, int msgWrite, int iPort)
{
	int ret;
	list<string>::iterator iter;
	ret = AdNetBaseInit(msgRead, msgWrite);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("NetBase Init Fail\n");
		return AD_FAILURE;
	}

	if(iPort > 0)
	{
		ret	= m_cTcpServer.TcpServerInit(iPort, false);
		if(ret != AD_SUCCESS)
		{
			AD_ERROR("Tcp Server Init Fail\n");
			return AD_FAILURE;
		}

		ret = AdNetBaseAddServer(m_cTcpServer.TcpServerGetSocket());
		if(ret != AD_SUCCESS)
		{
			AD_ERROR("Add Server Fail\n");
			return AD_FAILURE;
		}
	}

	return ret;
}

AdSession * AdMult::CreateSession(string & sServer)
{
	vector<string> vStr;
	int id;
	int ret;
	AdSession * pSession = NULL;

	AdString::StrSplit(sServer, vStr, ":");
	if(vStr.size() != 2)
	{
		return NULL;
	}

	pSession = AdNetBaseNewSession();
	if(pSession == NULL)
	{
		AD_ERROR("Memory not enough\n");
		return NULL;
	}

	id = pSession->AdSessionInit(vStr[0], atoi(vStr[1].c_str()));
	if(id <= 0)
	{
		AD_ERROR("Session Init Fail\n");
		AdNetBaseDeleteSession(pSession);
		return NULL;
	}

	ret = AdNetBaseAddClient(pSession);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("Add Server Fail\n");
		AdNetBaseDeleteSession(pSession);
		return NULL;
	}
	if(m_pPreSend)
		m_pPreSend(pSession);
	ctLock.lock();
	m_mSession[sServer] = pSession;
	m_mServer[pSession] = sServer;
	ctLock.unlock();
	return pSession;
}


int AdMult::DelSession(string & sServer)
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

int AdMult::DelSession(AdSession * pSession)
{
	map<AdSession *, string>::iterator iter;

	ctLock.lock();
	iter = m_mServer.find(pSession);
	if(iter == m_mServer.end())
	{
		ctLock.unlock();
		return AD_SUCCESS;
	}
	
	string & sServer = iter->second;
	pSession->AdSessionClose();
	m_mSession.erase(sServer);
	m_mServer.erase(iter);
	ctLock.unlock();
	return AD_SUCCESS;
}


AdSession *AdMult::GetSession(string & sServer)
{
	map<string, AdSession * >::iterator iter;

	ctLock.lock();
	iter = m_mSession.find(sServer);
	if(iter == m_mSession.end())
	{
		ctLock.unlock();
		return NULL;
	}
	
	AdSession *pSession = iter->second;
	ctLock.unlock();
	return pSession;
}



int AdMult::AdNetBaseSessionReconnect(AdSession * pSession)
{
	AdSession * pNew;
	map<AdSession * , string>::iterator iter;
	ctLock.lock();
	iter = m_mServer.find(pSession);
	if(iter != m_mServer.end())
	{
		string sServer = iter->second;
		pNew = CreateSession(sServer);
		if(pNew == NULL)
		{
			AD_ERROR("Try Create Session Fail\n");
			m_mSession.erase(iter->second);
			m_mServer.erase(iter);
		}
	}
	ctLock.unlock();
	if(pNew != NULL)
		return AD_SUCCESS;
	else
		return AD_FAILURE;
}

