/***************************************
*	Description: Ad config module,  config file parse and input.
*	Author: wilson
*	Date: 2016/01/05
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef	_AD_RTB_CONF_H
#define	_AD_RTB_CONF_H

#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <map>

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "AdGlobal.h"
#include "AdUtil.h"


using namespace std;

typedef  map<string, string>  ADCONFIG;
typedef  map<string, string>::iterator   ADCONFIG_ITER;

#define AD_CONFIG_MAX                   			512

class AdRtbConfig
{
public:
	AdRtbConfig();
	virtual ~AdRtbConfig();
	//load file content
	int Load(string & file);
	//check file timestam
	int Check();
	//clear module
	void Clear();

	//get Section Content
	ADCONFIG &  Get(string & Section);
	//check file changed
	bool FileIsChanged(string & sFile);
	bool FileIsChanged();

protected:
	//deleter invalid char
	int StrDeleteInvalid(string& str);
	//generate DevId By ip
	int  GetDevId(string & sId);
private:

	//Get Device Ip address
	int GetIp(string & sEth, string  & sIp);
	//config content
	map<string, ADCONFIG > m_mConfig;
	//invalid char 
	vector<char> m_vInvalidChar;
	//file timestamp, check file changed
	map<string, struct timespec> m_mFileTimeStamp;
	//changed file, need reload
	map<string, UINT>  m_mChangedFile;
	//config file name, full path
	string m_sConfigFile;
};
#endif 
