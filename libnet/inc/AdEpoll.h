/***************************************
*	Description: event module
*	Author: wilson
*	Date: 2015/09/07
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_EVENT_H__
#define __AD_EVENT_H__
#include <map>
#include <sys/epoll.h>
#include "AdGlobal.h"
#include "AdUtil.h"

using namespace std;

#define  AD_EVENT_READ                  0x1
#define  AD_EVENT_WRITE               0x2
#define  AD_EVENT_ERROR               0x4


class AdEvent
{
public:
	AdEvent();
	virtual ~AdEvent();

	int AdEventAdd(int iSocket, UINT uiFlag, UINT uiTimeout = 0);
	int AdEventModify(int iSocket, UINT uiFlag, UINT uiTimeout = 0);
	virtual int AdEventHandleMsg(int iSocket, int iMsgType);
	int AdEventDel(int iSocket);
	void AdEventWait(int iMilSeconds);
	int AdEventState(int iSocket);

private:
	int m_iEpoll;
	map<int, UINT> m_mState;
	map<int, UINT> m_mSockTimeout;
	map<UINT, int> m_mTimeoutSock;
};

#endif
