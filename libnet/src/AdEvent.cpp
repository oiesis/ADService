/***************************************
*	Description: event module, use libevent.
*	Author: wilson
*	Date: 2015/10/21
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "AdEvent.h"

void AdEventHandle(int iSock, short event, void *arg)
{
	AdEvent * pEvent = static_cast<AdEvent *>(arg);
	if(event & EV_READ)
	{
		pEvent->AdEventHandleMsg(iSock, AD_EVENT_READ);
	}
	else if(event & EV_TIMEOUT)
	{
		pEvent->AdEventHandleMsg(iSock, AD_EVENT_TIMEOUT);
	}
	if(event & EV_WRITE)
	{
		pEvent->AdEventHandleMsg(iSock, AD_EVENT_WRITE);
	}
}


AdEvent::AdEvent()
{
    	m_pEventBase=event_base_new();
	if (m_pEventBase == NULL) 
	{
		AD_ERROR("Event create Fail\n");
	}
}

AdEvent::~AdEvent()
{
	clear();	
}

void AdEvent::clear()
{
	map<int, struct event *>::iterator iter;
	
	m_mState.clear();
	m_mTimeout.clear();

	for(iter = m_mRead.begin(); iter != m_mRead.end(); iter ++)
	{
		event_del(iter->second);
		event_free(iter->second);
	}
	m_mRead.clear();

	for(iter = m_mWrite.begin(); iter != m_mWrite.end(); iter ++)
	{
		event_del(iter->second);
		event_free(iter->second);
	}
	m_mWrite.clear();

	//how to delele m_pEventBase
	if(m_pEventBase != NULL)
	{
		event_base_free(m_pEventBase);
		m_pEventBase = NULL;
	}
}

int AdEvent::AdEventState(int iSocket)
{
	if(m_mState.find(iSocket) == m_mState.end())
		return 0;
	return m_mState[iSocket];
}


int AdEvent::AdEventAdd(int iSocket, UINT uiFlag, UINT uiTimeout)
{	
	if((uiFlag & (AD_EVENT_READ | AD_EVENT_WRITE)) == 0)
	{
		return AD_FAILURE;
	}

	map<int, struct event *>::iterator iter;
	map<int, UINT>::iterator iter2;
	
	int uiOldFlag = m_mState[iSocket] ;
	
	if(uiTimeout > 0)
		m_mTimeout[iSocket] = uiTimeout;
	else
	{
		iter2 = m_mTimeout.find(iSocket);
		if(iter2 != m_mTimeout.end())
		{
			m_mTimeout.erase(iter2);
		}
	}

	struct timeval tv;
	
	struct event * pEvent;

	if(uiFlag & AD_EVENT_READ)
	{
		if((uiOldFlag & AD_EVENT_READ) == 0)
		{
			pEvent = event_new(m_pEventBase,iSocket, EV_ET | EV_READ | EV_PERSIST, AdEventHandle, this);
			if(pEvent == NULL)
			{
				AD_ERROR("Mem not enough\n");
				return AD_FAILURE;
			}
			if(uiTimeout > 0)
			{
				evutil_timerclear(&tv);
				tv.tv_sec = uiTimeout/1000;
				tv.tv_usec = (uiTimeout % 1000) * 1000;
				event_add(pEvent, &tv);
			}
			else
			{
				event_add(pEvent, NULL);
			}
			m_mRead[iSocket] = pEvent;
		}
		else
		{
			iter = m_mRead.find(iSocket);
			if((iter != m_mRead.end()) && (uiTimeout > 0))
			{
				pEvent = iter->second;
				evutil_timerclear(&tv);
				tv.tv_sec = uiTimeout/1000;
				tv.tv_usec = (uiTimeout % 1000) * 1000;
				event_add(pEvent, &tv);
			}
		}
	}
	else
	{
		if(uiOldFlag & AD_EVENT_READ)
		{
			iter = m_mRead.find(iSocket);
			if(iter != m_mRead.end())
			{
				event_del(iter->second);
				event_free(iter->second);
				m_mRead.erase(iSocket);
			}
		}
	}
	
	if(uiFlag & AD_EVENT_WRITE)
	{
		if((uiOldFlag & AD_EVENT_WRITE) == 0)
		{
			pEvent = event_new(m_pEventBase,iSocket, EV_ET | EV_WRITE, AdEventHandle, this);
			if(pEvent == NULL)
			{
				AD_ERROR("Mem not enough\n");
				return AD_FAILURE;
			}
			event_add(pEvent, NULL);
			m_mWrite[iSocket] = pEvent;
		}
	}
	else
	{
		if(uiOldFlag & AD_EVENT_WRITE)
		{
			iter = m_mRead.find(iSocket);
			if(iter != m_mRead.end())
			{
				event_del(iter->second);
				event_free(iter->second);
				m_mRead.erase(iSocket);
			}
		}
	}

	m_mState[iSocket] = uiFlag;
	return AD_SUCCESS;
}

int AdEvent::AdEventModify(int iSocket, UINT uiFlag, UINT uiTimeout)
{
	int ret = AD_SUCCESS;
	map<int, UINT>::iterator iter;
	if(uiFlag != m_mState[iSocket])
	{
		AdEventDel(iSocket);
		ret = AdEventAdd(iSocket, uiFlag, uiTimeout);
	}
	else
	{
		iter = m_mTimeout.find(iSocket);
		//find socket timeout
		if(iter != m_mTimeout.end())
		{
			//no same change it
			if(iter->second != uiTimeout)
			{
				if(uiTimeout == 0)
					AdEventDel(iSocket);
				ret = AdEventAdd(iSocket, uiFlag, uiTimeout);
			}
		}
		else
		{
			//no fond, first set
			if(uiTimeout > 0)
			{
				AdEventDel(iSocket);
				ret = AdEventAdd(iSocket, uiFlag, uiTimeout);
			}
		}
	}
	return ret;
}

int AdEvent::AdEventHandleMsg(int iSocket, int iMsgType)
{
	return 0;
}

int AdEvent::AdEventDel(int iSocket)
{
	map<int, struct event *>::iterator iter;
	
	iter = m_mRead.find(iSocket);
	if(iter != m_mRead.end())
	{
		event_del(iter->second);
		event_free(iter->second);
		m_mRead.erase(iSocket);
	}

	iter = m_mWrite.find(iSocket);
	if(iter != m_mWrite.end())
	{
		event_del(iter->second);
		event_free(iter->second);
		m_mWrite.erase(iSocket);
	}
	m_mState.erase(iSocket);
	m_mTimeout.erase(iSocket);
	return AD_SUCCESS;
}

void AdEvent::AdEventWait(int iMilSeconds)
{
	event_base_loop(m_pEventBase, EVLOOP_NONBLOCK);
	//event_base_dispatch(m_pEventBase);
}
