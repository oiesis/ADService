/***************************************
*	Description: event module, use libevent.
*	Author: wilson
*	Date: 2015/10/21
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_EVENT_H__
#define __AD_EVENT_H__
#include <map>
#include "event2/event.h"
#include "AdGlobal.h"
#include "AdUtil.h"

using namespace std;

#define  AD_EVENT_READ                  0x1
#define  AD_EVENT_WRITE               0x2
#define AD_EVENT_TIMEOUT          0x4
#define  AD_EVENT_ERROR               0x8


class AdEvent
{
public:
	AdEvent();
	virtual ~AdEvent();

	int AdEventAdd(int iSocket, UINT uiFlag,  UINT uiTimeout = 0);
	int AdEventModify(int iSocket, UINT uiFlag, UINT uiTimeout = 0);
	virtual int AdEventHandleMsg(int iSocket, int iMsgType);
	int AdEventDel(int iSocket);
	void AdEventWait(int iMilSeconds);
	int AdEventState(int iSocket);
	void clear();

private:
	struct event_base*  m_pEventBase;
	map<int, struct event *> m_mWrite;
	map<int, struct event *> m_mRead;
	map<int, UINT> m_mState;
	map<int, UINT> m_mTimeout;   
};

#endif

