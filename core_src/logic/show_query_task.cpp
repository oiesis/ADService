//
// Created by guoze.lin on 16/4/29.
//

#include "show_query_task.h"
#include "adselect/core_adselect_manager.h"
#include "adselect/ad_select_logic.h"
#include "atomic.h"

namespace adservice{
    namespace corelogic{

        static long handleShowRequests = 0;
        static long updateShowRequestsTime = 0;

        int HandleShowQueryTask::initialized = 0;
        char HandleShowQueryTask::showAdxTemplate[1024];
        char HandleShowQueryTask::showSspTemplate[1024];

        void HandleShowQueryTask::loadTemplates(){
            if(initialized==1||!ATOM_CAS(&initialized,0,1))
                return;
            loadFile(showAdxTemplate,TEMPLATE_SHOW_ADX_PATH);
            loadFile(showSspTemplate,TEMPLATE_SHOW_SSP_PATH);
        }

        void HandleShowQueryTask::filterParamMapSafe(ParamMap &paramMap) {
            AbstractQueryTask::filterParamMapSafe(paramMap);
            if(paramMap.find(URL_CREATIVE_ID)!=paramMap.end())
                stringSafeNumber(paramMap[URL_CREATIVE_ID]);
            if(paramMap.find(URL_SSP_PID)!=paramMap.end())
                stringSafeNumber(paramMap[URL_SSP_PID]);
        }

        /**
         * 因为业务数据库数据插入ElasticSearch时会对引号进行转义
         * 此函数用于反转义引号
         */
        void tripslash(char* str){
            char* p1 = str,*p2=p1;
            while(*p2!='\0'){
                if(*p2=='\\'&&p2[1]=='\''){
                    p2++;
                }
                *p1++ = *p2++;
            }
            *p1='\0';
        }

        /**
         * 判断曝光的类型
         * of=0 DSP,此为默认的情况
         * of=1 SSP
         * of=2 移动展示
         * of=3 移动日志
         */
        bool isShowForSSP(ParamMap& paramMap){
            ParamMap::const_iterator iter = paramMap.find(URL_IMP_OF);
            if(iter==paramMap.end()){
                return false;
            }else{
                return iter->second == OF_SSP;
            }
        }

        /**
         * 判断是否移动曝光
         */
        bool isShowForMobile(ParamMap& paramMap){
            ParamMap::const_iterator iter = paramMap.find(URL_IMP_OF);
            if(iter==paramMap.end()){
                return false;
            }else{
                return iter->second == OF_DSP_MOBILE_SHOW || iter->second == OF_DSP_MOBILE_LOG;
            }
        }

        /**
         * 根据OF类型判定是否需要显示创意和纪录日志,http://redmine.mtty.com/redmine/issues/48
         */
        void dspSetParam(ParamMap& paramMap,bool& showCreative,bool& shouldLog){
            ParamMap::const_iterator iter = paramMap.find(URL_IMP_OF);
            if(iter==paramMap.end() || iter->second==OF_DSP){
                showCreative = true;
                shouldLog =true;
            }else if(iter->second == OF_DSP_MOBILE_SHOW){
                showCreative = true;
                shouldLog = false;
            }else if(iter->second == OF_DSP_MOBILE_LOG){
                showCreative = false;
                shouldLog = true;
            }
        }

#define MakeStringValue(s) rapidjson::Value().SetString(s.c_str(),s.length())

#define MakeStringConstValue(s) rapidjson::Value().SetString(s)

#define MakeIntValue(s) rapidjson::Value().SetInt(s)

#define MakeDoubleValue(d) rapidjson::Value().SetDouble(d)

#define MakeBooleanValue(b) rapidjson::Value().SetBool(b)

        /**
         * 使用生成JSON对象的方法,为前端准备DSP曝光内容
         * 目前没有使用
         */
        void prepareMtAdInfoForDSP(rapidjson::Document& mtAdInfo,ParamMap& paramMap,rapidjson::Value& result){
            rapidjson::Document::AllocatorType& allocator = mtAdInfo.GetAllocator();
            mtAdInfo.SetObject();
            mtAdInfo.AddMember("pid",MakeStringValue(paramMap[URL_ADPLACE_ID]),allocator);
            mtAdInfo.AddMember("width",MakeIntValue(json::getField(result,"width",250)),allocator);
            mtAdInfo.AddMember("height",MakeIntValue(json::getField(result,"height",300)),allocator);
            mtAdInfo.AddMember("impid",MakeStringValue(paramMap[URL_EXPOSE_ID]),allocator);
            mtAdInfo.AddMember("advid",MakeStringValue(paramMap[URL_ADOWNER_ID]),allocator);
            mtAdInfo.AddMember("unid",MakeStringValue(paramMap[URL_ADX_ID]),allocator);
            mtAdInfo.AddMember("plid",MakeStringConstValue(""),allocator);
            mtAdInfo.AddMember("gpid",MakeStringValue(paramMap[URL_EXEC_ID]),allocator);
            mtAdInfo.AddMember("cid",MakeStringValue(paramMap[URL_CREATIVE_ID]),allocator);
            mtAdInfo.AddMember("arid",MakeStringValue(paramMap[URL_AREA_ID]),allocator);
            mtAdInfo.AddMember("ctype",MakeIntValue(json::getField(result,"banner_type",1)),allocator);
            mtAdInfo.AddMember("xcurl",MakeStringValue(paramMap[URL_ADX_MACRO]),allocator);
            mtAdInfo.AddMember("tview",MakeStringConstValue(""),allocator);
            rapidjson::Value mtls(kArrayType);
            rapidjson::Value mtlsObj(kObjectType);
            mtlsObj.AddMember("p0",MakeStringValue(json::getField(result,"material_url","")),allocator);
            mtlsObj.AddMember("p1",MakeStringValue(json::getField(result,"click_url","")),allocator);
            mtlsObj.AddMember("p2",MakeStringConstValue("000"),allocator);
            mtlsObj.AddMember("p3",MakeIntValue(json::getField(result,"width",250)),allocator);
            mtlsObj.AddMember("P4",MakeIntValue(json::getField(result,"height",300)),allocator);
            mtlsObj.AddMember("p5",MakeStringConstValue(""),allocator);
            mtlsObj.AddMember("p6",MakeStringConstValue(""),allocator);
            mtlsObj.AddMember("p7",MakeStringConstValue(""),allocator);
            mtlsObj.AddMember("p8",MakeStringConstValue(""),allocator);
            mtls.PushBack(mtlsObj.Move(),allocator);
            mtAdInfo.AddMember("mtls",mtls.Move(),mtAdInfo.GetAllocator());
        }

        /**
         * 使用生成JSON对象的方法,为前端准备曝光内容
         * 目前没有使用
         */
        void fillHtmlUnFixedParam(rapidjson::Document& mtAdInfo,ParamMap& paramMap,rapidjson::Value& result){
            const std::string binder="%s";
            if(binder == mtAdInfo["pid"].GetString()){
                mtAdInfo["pid"] = MakeStringValue(paramMap[URL_ADPLACE_ID]);
            }
            if(binder == mtAdInfo["width"].GetString()){
                mtAdInfo["width"] = MakeIntValue(json::getField(result,"width",0));
            }
            if(binder == mtAdInfo["height"].GetString()){
                mtAdInfo["height"] = MakeIntValue(json::getField(result,"height",0));
            }
            if(binder == mtAdInfo["impid"].GetString()) {
                mtAdInfo["impid"] = MakeStringValue(paramMap[URL_EXPOSE_ID]);
            }
            if(binder == mtAdInfo["advid"].GetString()) {
                mtAdInfo["advid"] = MakeStringValue(paramMap[URL_ADOWNER_ID]);
            }
            if(binder == mtAdInfo["unid"].GetString()) {
                mtAdInfo["unid"]  = MakeStringValue(paramMap[URL_ADX_ID]);
            }
            if(binder == mtAdInfo["plid"].GetString()) {
                mtAdInfo["plid"] =  MakeStringConstValue("");
            }
            if(binder == mtAdInfo["gpid"].GetString()) {
                mtAdInfo["gpid"] = MakeStringValue(paramMap[URL_EXEC_ID]);
            }
            if(binder == mtAdInfo["cid"].GetString()) {
                mtAdInfo["cid"] = MakeStringValue(paramMap[URL_CREATIVE_ID]);
            }
            if(binder == mtAdInfo["arid"].GetString()) {
                mtAdInfo["arid"] = MakeStringValue(paramMap[URL_AREA_ID]);
            }
            if(binder == mtAdInfo["ctype"].GetString()) {
                mtAdInfo["ctype"] = MakeIntValue(json::getField(result, "banner_type", 1));
            }
            if(binder == mtAdInfo["xcurl"].GetString()) {
                mtAdInfo["xcurl"] = MakeStringValue(paramMap[URL_ADX_MACRO]);
            }
            if(binder == mtAdInfo["tview"].GetString()){
                mtAdInfo["tview"] = MakeStringConstValue("");
            }
        }

        /**
         * 根据URL参数,数据库创意json,模板等已有信息生成完备的曝光页面
         * 此方法使用固定参数位置绑定的方法进行内容填充
         * 目前用于生成DSP曝光页面
         * @param paramMap: URL 参数表
         * @param html: 数据库创意json模板,由于历史问题,这是不满足RFC规范的JSON
         * @param templateFmt:返回页面的模板
         * @param buffer: 最终结果的存放位置
         * @param bufferSize: buffer的大小
         */
        int fillHtmlFixedParam(ParamMap& paramMap,const char* html,const char* templateFmt,char* buffer,int bufferSize){
            char mtAdInfo[4096];
            char adxbuffer[1024];
            std::string adxMacro;
            urlDecode_f(paramMap[URL_ADX_MACRO],adxMacro,adxbuffer);
            adxMacro+=ADX_MACRO_SUFFIX;
            //这里因为使用了C++的字符串模板进行参数绑定,而获取的模板中有可能带有混淆字符,所以需要分解处理
            //更好的解决方法还是使用json parse的方式,不过要求html的json格式按照RFC规范
            char* mtlsPos = (char*)strstr(html,"mtls");
            char backupChar = '\0';
            if(mtlsPos!=NULL){
                backupChar = *mtlsPos;
                *mtlsPos = '\0';
            }
            int len=snprintf(mtAdInfo,sizeof(mtAdInfo),html,paramMap[URL_ADPLACE_ID].c_str(),paramMap[URL_EXPOSE_ID].c_str(),paramMap[URL_ADX_ID].c_str(),
                             "",paramMap[URL_EXEC_ID].c_str(),paramMap[URL_AREA_ID].c_str(),adxMacro.c_str());
            if(mtlsPos!=NULL){
                *mtlsPos = backupChar;
                if(len>=sizeof(mtAdInfo)){
                    DebugMessageWithTime("mtAdInfo length too long,length:",len);
                    len = sizeof(mtAdInfo)-1;
                }
                strncpy(mtAdInfo+len,mtlsPos,sizeof(mtAdInfo)-len-1);
                mtAdInfo[sizeof(mtAdInfo)-1]='\0';
            }
            tripslash(mtAdInfo);
            len = snprintf(buffer,bufferSize-1,templateFmt,mtAdInfo);
            if(len>=bufferSize){
                len = bufferSize - 1;
            }
            return len;
        }

        /**
         * 根据ADSELECT返回的创意(banner),广告位(adplace),投放单信息(solution),模板等已有信息生成完备的曝光页面
         * 此方法使用固定参数位置绑定的方法进行内容填充
         * 目前用于生成SSP曝光页面
         * @param solution: 投放单
         * @param adplace : 广告位
         * @param banner : 创意
         * @param paramMap: URL 参数
         * @param html : 创意json模板,由于历史问题,这是不满足RFC规范的JSON
         * @param templateFmt:返回页面的模板
         * @param buffer: 最终结果的存放位置
         * @param bufferSize: buffer的大小
         */
        int fillHtmlFixedParam(rapidjson::Value& solution,rapidjson::Value& adplace,rapidjson::Value& banner,ParamMap& paramMap,const char* html,
                               const char* templateFmt,char* buffer,int bufferSize){
            char mtAdInfo[4096];
            char* mtlsPos = (char*)strstr(html,"mtls");
            char backupChar = '\0';
            if(mtlsPos!=NULL){
                backupChar = *mtlsPos;
                *mtlsPos = '\0';
            }
            int len=snprintf(mtAdInfo,sizeof(mtAdInfo),html,to_string(adplace["pid"].GetInt()).c_str(),cypher::randomId(5).c_str(),to_string(adplace["adxid"].GetInt()).c_str(),
                             "",to_string(solution["sid"].GetInt()).c_str(),"0086-ffff-ffff","");
            if(mtlsPos!=NULL){
                *mtlsPos = backupChar;
                if(len>=sizeof(mtAdInfo)){
                    DebugMessageWithTime("mtAdInfo length too long,length:",len);
                    len = sizeof(mtAdInfo)-1;
                }
                strncpy(mtAdInfo+len,mtlsPos,sizeof(mtAdInfo)-len-1);
                mtAdInfo[sizeof(mtAdInfo)-1]='\0';
            }
            tripslash(mtAdInfo);
            len = snprintf(buffer,bufferSize-1,templateFmt,paramMap["callback"].c_str(),mtAdInfo);
            if(len>=bufferSize){
                len = bufferSize -1;
            }
            return len;
        }



        void HandleShowQueryTask::customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& response){
            bool isSSP = isShowForSSP(paramMap);
            const char* templateFmt = isSSP?showSspTemplate:showAdxTemplate;
            //连接ADSelect
            AdSelectManager& adselect = AdSelectManager::getInstance();
            int seqId = 0;
            CoreModule coreModule = CoreService::getInstance();
            if(coreModule.use_count()>0){
                seqId = coreModule->getExecutor().getThreadSeqId();
            }
            std::string respBody;
            if(isSSP){//SSP
                std::string& queryPid = paramMap[URL_SSP_PID];
                bool isAdxPid = false;
                if(queryPid.empty()||queryPid=="0"){
                    queryPid = paramMap[URL_SSP_ADX_PID];
                    if(queryPid.empty()){ //URL 参数有问题
                        DebugMessageWithTime("in show module,ssp url pid is empty");
                        return;
                    }
                    isAdxPid = true;
                }
                AdSelectLogic adSelectLogic(&adselect);
                if(!adSelectLogic.selectByPid(seqId,queryPid,isAdxPid)){
                    log.adInfo.pid = isAdxPid?"0":queryPid;
                    log.adInfo.adxpid = isAdxPid?queryPid:"0";
                    log.reqStatus = 500;
                    return;
                }
                const SelectResult& selectResult = adSelectLogic.getResult();
                rapidjson::Value& banner = *(selectResult.banner);
                rapidjson::Value& finalSolution = *(selectResult.finalSolution);
                rapidjson::Value& adplace = *(selectResult.adplace);
                log.adInfo.bannerId = banner["bid"].GetInt();
                log.adInfo.advId = finalSolution["advid"].GetInt();
                log.adInfo.adxid = adplace["adxid"].GetInt();
                log.adInfo.pid = to_string(adplace["pid"].GetInt());
                log.adInfo.sid = finalSolution["sid"].GetInt();
                log.adInfo.mid = adplace["mid"].GetInt();
                log.adInfo.cid = adplace["cid"].GetInt();
                log.adInfo.bidPrice = selectResult.bidPrice;
                log.adInfo.cost = adplace["costprice"].GetInt();
                const char* tmp = banner["html"].GetString();
                //返回结果
                char buffer[8192];
                int len = fillHtmlFixedParam(finalSolution,adplace,banner,paramMap,tmp,templateFmt,buffer,sizeof(buffer));
                respBody = std::string(buffer,buffer+len);
            }else {//DSP of=0,of=2,of=3
                bool showCreative = true;
                dspSetParam(paramMap,showCreative,needLog);
                rapidjson::Document esResp(rapidjson::kObjectType);
                rapidjson::Value &result = adselect.queryCreativeByIdCache(seqId, paramMap[URL_CREATIVE_ID], esResp);
                if (!result.HasMember("html")) {
                    log.adInfo.bannerId = 0;
                    log.reqStatus = HttpResponse::k500ServerError;
                    return;
                }
                if(showCreative) { //需要显示创意
                    const char *tmp = result["html"].GetString();
                    char buffer[8192];
                    int len = fillHtmlFixedParam(paramMap, tmp, templateFmt, buffer,sizeof(buffer));
                    respBody = std::string(buffer, buffer + len);
                }
            }
#ifdef USE_ENCODING_GZIP
            Buffer body;
                muduo::net::ZlibOutputStream zlibOutStream(&body);
                zlibOutStream.write(respBody);
                zlibOutStream.finish();
                response.setBody(body.retrieveAllAsString());
#else
            response.setBody(respBody);
#endif
            response.addHeader("Pragma", "no-cache");
            response.addHeader("Cache-Control", "no-cache,no-store;must-revalidate");
            response.addHeader("P3p",
                               "CP=\"CURa ADMa DEVa PSAo PSDo OUR BUS UNI PUR INT DEM STA PRE COM NAV OTC NOI DSP COR\"");
            handleShowRequests++;
            if (handleShowRequests % 10000 == 1) {
                int64_t todayStartTime = time::getTodayStartTime();
                if (updateShowRequestsTime < todayStartTime) {
                    handleShowRequests = 1;
                    updateShowRequestsTime = todayStartTime;
                } else {
                    DebugMessageWithTime("handleShowRequests:", handleShowRequests);
                }
            }
        }

    }
}
