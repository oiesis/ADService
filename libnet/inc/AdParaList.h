/***************************************
*	Description: Parameter list
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#ifndef   __AD_PARALIST_H__
#define __AD_PARALIST_H__
#include <list>
#include <vector>
#include "AdTask.h"
#include "AdBuff.h"


template<typename Request, typename Master>
class AdParaList
{
public:
	AdParaList(){};
	virtual ~AdParaList(){};

	int clear(Master * pMas, AdTask & ctTask)
	{
		typename  std::map<Request *, ULONG>::iterator ltIter;
		Request * pReq;
		AdSession * pSrcSession;
		AdSession * pDstSession;
		
		m_ctLock.lock();
		for(ltIter =m_mReq.begin(); ltIter != m_mReq.end(); ltIter ++)
		{
			pReq = ltIter->first;
			pDstSession = static_cast<AdSession * >(pReq->m_pDstSession);
			if(pReq->m_bTimeout == false)
			{		 
				pReq->m_bTimeout = true;		
				pSrcSession = static_cast<AdSession * >(pReq->m_pSrcSession);
				if(pMas != NULL)
					pMas->SessionTimeout(*pReq, ctTask, pSrcSession);
			}
			pMas->FreePara(pReq);
			pDstSession->AdSessionIdle();	
		}
		m_mReq.clear();
		m_mTime.clear();
		m_ctLock.unlock();
		return AD_SUCCESS;
	};

	
	int check(Master * pMas, AdTask & ctTask, bool bDel = false)
	{
		typename std::map<ULONG, Request *>::iterator ltIter;
		Request * pReq;
		AdSession * pSrcSession;
		int iCount = 0;
		m_ctLock.lock();
		
		ltIter =m_mTime.begin();
		while(ltIter != m_mTime.end())
		{
			 pReq = ltIter->second;
			 if(pReq->m_bTimeout)
			 {
			 	if(bDel)
				{
					m_mReq.erase(ltIter->second);
					m_mTime.erase(ltIter++);
			 	}
				continue;
			 }
			 if(pReq->m_ctRequest.checkLife())
			{
				iCount ++;
				pReq->m_bTimeout = true;
				pSrcSession = static_cast<AdSession *>(pReq->m_pSrcSession);
				if(pMas != NULL)
					pMas->SessionTimeout(*pReq, ctTask, pSrcSession);

				if(bDel)
				{
					pMas->FreePara(pReq);
					m_mReq.erase(ltIter->second);
					m_mTime.erase(ltIter++);
			 	}
			}
			else
				break;
		}
		m_ctLock.unlock();
		return iCount;
	};

	int push(Request * pReq)
	{
		m_ctLock.lock();
		m_mReq[pReq] = pReq->m_ctRequest.getTime();
		m_mTime[pReq->m_ctRequest.getTime()] = pReq;
		m_ctLock.unlock();
		return AD_SUCCESS;
	};

	int del(Request * pReq)
	{
		typename std::map<Request *, ULONG>::iterator ltIter;
		int iDel = AD_FAILURE;
		m_ctLock.lock();
		ltIter = m_mReq.find(pReq);
		if(ltIter != m_mReq.end())
		{
			m_mTime.erase(ltIter->second);
			m_mReq.erase(ltIter);
			iDel = AD_SUCCESS;
		}
		m_ctLock.unlock();
		return iDel;
	};

	Request * del(ULONG ulTime)
	{
		typename std::map<ULONG, Request *>::iterator ltIter;
		Request * pReq = NULL;
		m_ctLock.lock();
		ltIter = m_mTime.find(ulTime);
		if(ltIter != m_mTime.end())
		{
			pReq = ltIter->second;
			m_mReq.erase(ltIter->second);
			m_mTime.erase(ltIter);
		}
		m_ctLock.unlock();
		return pReq;
	};

	Request *  pop()
	{
		Request * pReq = NULL;
		typename std::map<ULONG, Request *>::iterator ltIter;
		m_ctLock.lock();
		ltIter = m_mTime.begin();
		if(ltIter != m_mTime.end())
		{
			pReq = ltIter->second;
			m_mReq.erase(ltIter->second);
			m_mTime.erase(ltIter);
		}
		m_ctLock.unlock();
		return pReq;
	};
	
	int size()
	{
		int cnt;
		m_ctLock.lock();
		cnt = m_mReq.size();
		m_ctLock.unlock();
		return cnt;
	}

private:
	std::map<Request *, ULONG> m_mReq;
	std::map<ULONG, Request *> m_mTime;
	LockMutex m_ctLock;
};

#endif

