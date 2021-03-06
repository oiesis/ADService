//
// Created by guoze.lin on 16/5/4.
//

#include "ad_select_logic.h"
#include "constants.h"
#include "utility.h"


namespace adservice{
    namespace adselect{


        /**
         * 从创意组的多个创意中随机选择一个创意
         */
        rapidjson::Value& selectBannerFromBannerGroup(std::map<int,std::vector<rapidjson::Value*>>& bannerMap,int bgid){
            std::vector<rapidjson::Value*>& banners = bannerMap[bgid];
            int r = std::abs(rng::randomInt())%banners.size();
            return *(banners[r]);
        }

        /**
         * 从创意组的多个创意中选择最好的创意,根据需求即出价(offerprice)高的创意
         */
        rapidjson::Value& bestBannerFromBannerGroup(std::map<int,std::vector<rapidjson::Value*>>& bannerMap,int bgid){
            std::vector<rapidjson::Value*>& banners = bannerMap[bgid];
            int idx[50];
            int idxCnt=0,i=0;
            double maxScore = -1.0;
            typedef std::vector<rapidjson::Value*>::iterator Iter;
            for(Iter iter = banners.begin();iter!=banners.end();iter++,i++){
                rapidjson::Value& banner = *(*iter);
                double ecpm = banner["offerprice"].GetDouble();
                if(ecpm>maxScore){
                    idxCnt = 0;
                    idx[idxCnt++] = i;
                    maxScore = ecpm;
                }else if(ecpm==maxScore){
                    idx[idxCnt++] = i;
                }
            }
            int r = std::abs(rng::randomInt())%idxCnt;
            return *(banners[idx[r]]);
        }

        /**
         * 为每个投放单根据价格类型和ecpm生成评分
         */
        double calcSolutionScore(int priceType,double ecpm){
            return (10-priceType)*10000+ecpm;
        }

        /**
         * 从评分中逆向求出ecpm值
         */
        double getECPMFromSolutionScore(double score,int priceType){
            return score - (10 - priceType)*10000;
        }

        /**
         * 根据投放单评分对投放单排序
         */
        void sortSolutionScore(int* idx,double* scores,int size){
            std::sort<int*>(idx,idx+size,[scores](const int& a,const int&b)->bool{
                return scores[a] > scores[b];
            });
        }

        /**
         * 根据权重随机选一个投放单
         */
        int randomSolution(double totalWeight,double* accWeights,int size){
            double r = std::abs(rng::randomDouble()) * totalWeight;
            int l=0,h=size-1;
            while(l<=h){
                int mid = l+((h-l)>>1);
                if(r<=accWeights[mid])
                    h = mid-1;
                else
                    l = mid+1;
            }
            assert(l>=0&&l<size);
            return l;
        }

        /**
         * 计算花费
         * 参见http://redmine.mtty.com/redmine/issues/22
         */
        double calcBidPrice(int priceType,double ecpm1OfferPrice,double ecpm2OfferPrice,double basePrice,double offerPrice,bool fromSSP){
            switch(priceType){
                case PRICETYPE_CPD:
                case PRICETYPE_CPM:
                case PRICETYPE_CPC:
                    return 0;
                case PRICETYPE_RCPC: //offerprice,定价采购流量
                    return std::max(offerPrice,basePrice);
                case PRICETYPE_RTB:
                case PRICETYPE_RRTB_CPM:
                    if(!fromSSP){ //ADX
                        offerPrice *= AD_RTB_BIDOFFERPRICE_FACTOR;
                        offerPrice=offerPrice<1.0?1.0:offerPrice;
                        return std::max(offerPrice,basePrice);
                    }else {//SSP
                        if (ecpm2OfferPrice < 0) { //只有一个出价
                            return std::max(ecpm1OfferPrice, basePrice);
                        } else { // SSP流量,第二高价加一分钱
                            return std::max(ecpm2OfferPrice + 1, basePrice);
                        }
                    }
                case PRICETYPE_RRTB_CPC:
                    return 0;
            }
            return 0;
        }


        bool filterSolutionField(const rapidjson::Value& solution,const char* fieldName,const std::string& toMatch){
            std::string fieldValue = solution[fieldName].IsNull()?"":solution[fieldName].GetString();
            if(fieldValue.empty()||fieldValue=="0"||toMatch.empty())
                return true;
            if(fieldValue.find(toMatch)!=std::string::npos){
                return true;
            }
            return false;
        }

        inline bool filterSolutionField(const rapidjson::Value& solution,const char* fieldName,int toMatch){
            return filterSolutionField(solution,fieldName,std::to_string(toMatch));
        }

        inline int countryGeo(int geo){
            return geo - (geo%AREACODE_MARGIN);
        }

#define ASSERT_SOLUTION_CONDITION(solution,field,match,boolValue) {   \
            if(filterSolutionField(solution,field,match)!=boolValue){  \
                return false;   \
            }   \
        }

        bool filterSolutionMultiCondition(bool simpleQuery,const rapidjson::Value& solution,const AdSelectCondition& condition){
            if(!simpleQuery){ //复杂查询上下文,solution已经经过复杂条件过滤,无需再过滤
                return true;
            }
            //后期可以直接走配置
            ASSERT_SOLUTION_CONDITION(solution,"d_adexchange",condition.adxid,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_mediatype", condition.mediaType,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_adplace", condition.mttyPid,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_adplacetype", condition.adplaceType,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_displaynumber", condition.displayNumber,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_flowtype", condition.flowType,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_hour", condition.dHour,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_geo",condition.dGeo,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_geo",countryGeo(condition.dGeo),true);
            ASSERT_SOLUTION_CONDITION(solution,"d_device",condition.mobileDevice,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_os",condition.pcOS,true);
            ASSERT_SOLUTION_CONDITION(solution,"d_dealid",condition.dealId,true);
            ASSERT_SOLUTION_CONDITION(solution,"n_adplace",condition.mttyPid,false);
            ASSERT_SOLUTION_CONDITION(solution,"n_geo",condition.dGeo,false);
            return true;
        }


        /**
         * 过滤IP
         */
        bool filterSolutionIp(const rapidjson::Value& solution,const std::string& ip){
            std::string dIp = solution["d_ip"].IsNull()?"":solution["d_ip"].GetString();
            if(dIp.empty()||dIp=="0"||ip.empty()){
                return true;
            }
            if(dIp.find(ip)!=std::string::npos){
                return true;
            }
            return false;
        }

        /**
         * 在走正常投放流程情况下,单子能投出来但需要做几率过滤
         */
        bool filterSolutionSuccessRate(const rapidjson::Value& solution){
            int controlType = solution["budget_share"].GetInt();
            int successRate = solution["rate"].GetInt();
            if(controlType==SOLUTION_CONTROLL_TYPE_ACC&&successRate>0) //加速投放不作几率控制
                return true;
            int r = std::abs(rng::randomInt())%SOLUTION_SUCCESS_RATE_BASE;
            return r<successRate;
        }

        void split(const char* str,int len,const char** index,int& size){
            index[0] = str;
            int i =0,j=1;
            for(;i<len&&j<size;i++){
                if(str[i]=='|'){
                    index[j]=str+i+1;
                    j++;
                }
            }
            if(j<size)
                size = j;
            if(size%3!=0){
                DebugMessageWithTime("error in split,size not times of 3,input:",str);
            }
        }

        bool extractConditionOption(std::string& condition,std::string& filter,double& offerPrice,double& ctr){
            const char* index[3000];
            int size = sizeof(index)/sizeof(char*);
            split(condition.data(),condition.length(),index,size);
            for(int i=1;i<=size;i+=3){
                if(!strncmp(index[i],filter.data(),filter.length())){
                    offerPrice+=atof(index[i+1]);
                    ctr+=atof(index[i+2]);
                    return true;
                }
            }
            return false;
        }

        /**
         * 根据定向类型对基本出价和ctr进行加成
         */
        void addupPricer(const rapidjson::Value& solution,AdSelectCondition& selectCondition,double& offerPrice,double& ctr){
            //地域投放 溢价 http://redmine.mtty.com/redmine/issues/100
            std::string dGeo = solution["d_geo"].GetString();
            if(dGeo != "0"){
                std::string geo = std::to_string(selectCondition.dGeo);
                if(!extractConditionOption(dGeo,geo,offerPrice,ctr)){//省市通投匹配
                    std::string countryGeo = std::to_string(selectCondition.dGeo-(selectCondition.dGeo%AREACODE_MARGIN));
                    extractConditionOption(dGeo,countryGeo,offerPrice,ctr);//国家级通投匹配
                }
            }
            //广告位维度 溢价 http://redmine.mtty.com/redmine/issues/100
            std::string dAdplace = solution["d_adplace"].GetString();
            if(dAdplace!="0"){
                std::string pid = selectCondition.mttyPid;
                extractConditionOption(dAdplace,pid,offerPrice,ctr);
            }
        }

        /**
         * 获取hour比例
         */
        double getHourPercent(const rapidjson::Value& solution,std::string& dHour){
            if(dHour.empty()){
                //DebugMessageWithTime("input dHour is empty");
                return 1.0;
            }
            std::string hour = solution["d_hour"].GetString();
            if(hour=="0")
                return 1.0;
            const char* index[3000];
            int size = sizeof(index)/sizeof(char*);
            split(hour.data(),hour.length(),index,size);
            for(int i=1;i<=size;i+=3){
                if(!strncmp(index[i],dHour.data(),dHour.length())){
                    return atof(index[i+1]);
                }
            }
            return 1.0;
        }

        bool AdSelectLogic::selectByCondition(int seqId, AdSelectCondition &condition, bool isAdxPid,bool fromSSP,bool simpleQuery) {
            rapidjson::Value& result = adselect->queryAdInfoByCondition(seqId,condition,esResp,isAdxPid,simpleQuery);
            if(!result.IsArray()||result.Empty()){ //正常情况下应该刷出solution和banner以及相应的高级出价器列表
                return false;
            }
            //从返回结果中整理数据
            rapidjson::Value* padplace = NULL;
            rapidjson::Value* solutions[100];
            int solCnt = 0;
            std::map<int,std::vector<rapidjson::Value*>> bannerMap;
            rapidjson::Value* superPricer =NULL;
            for(int i=0;i<result.Size();i++){
                const char* type = result[i]["_type"].GetString();
                if(!strcmp(type,ES_DOCUMENT_SOLUTION)){ //投放单
                    solutions[solCnt]=&(result[i]["_source"]);
                    if(filterSolutionMultiCondition(simpleQuery,*(solutions[solCnt]),condition)
                       && filterSolutionIp(*(solutions[solCnt]),condition.ip)  //IP过滤
                       && filterSolutionSuccessRate(*(solutions[solCnt]))){ //SuccessRate过滤
                        solCnt++;
                    }
                }else if(!strcmp(type,ES_DOCUMENT_BANNER)){ // 创意
                    rapidjson::Value& banner = result[i]["_source"];
                    int bgid = banner["bgid"].GetInt();
                    if(bannerMap.find(bgid)==bannerMap.end()){
                        bannerMap.insert(std::make_pair(bgid,std::vector<rapidjson::Value*>()));
                    }
                    std::vector<rapidjson::Value*>& bannerVec = bannerMap[bgid];
                    bannerVec.push_back(&banner);
                }else if(!strcmp(type,ES_DOCUMENT_ES_ADPLACE)){ //高级出价器,一个广告位的高级出价器有且只能有一个
                    superPricer = &(result[i]["_source"]);
                }else if(!strcmp(type,ES_DOCUMENT_ADPLACE)){//广告位
                    padplace = &(result[i]["_source"]);
                }
            }
            rapidjson::Value& adplace = *padplace;
            if(solCnt==0){ //失败
                DebugMessageWithTime("solutionCnt==0");
                return false;
            }
            //计算投放单ecpm,并计算score进行排序
            int solIdx[100];
            double finalOfferPrice[100];
            double solScore[100];
            double top1Ecpm = -1,top2Ecpm = -1;
            double top1EcpmOfferPrice = -1.0,top2EcpmOfferPrice = -1.0;
            for(int i=0;i<solCnt;i++){
                solIdx[i] = i;
                rapidjson::Value& solution = *(solutions[i]);
                double offerprice = solution["offerprice"].GetDouble();
                double ctr = solution["ctr"].GetDouble();
                if(superPricer && (*superPricer)["sid"].GetInt()==solution["sid"].GetInt()){
                    offerprice+=(*superPricer)["offerprice"].GetDouble();
                    ctr+=(*superPricer)["ctr"].GetDouble();
                }
                int bgid = solution["bgid"].GetInt();
                rapidjson::Value& banner = bestBannerFromBannerGroup(bannerMap,bgid);//advId?
                //创意维度 溢价 http://redmine.mtty.com/redmine/issues/101
                offerprice += banner["offerprice"].GetDouble();
                ctr += banner["ctr"].GetDouble();
                int priceType = solution["pricetype"].GetInt();
                double hourPercent = getHourPercent(solution,condition.dHour);
                addupPricer(solution,condition,offerprice,ctr);
                double ecpm = offerprice * (priceType == PRICETYPE_RRTB_CPC ?ctr: 1.0) * hourPercent;
                finalOfferPrice[i] = ecpm;
                solScore[i] = calcSolutionScore(priceType,ecpm);
                if(ecpm>top1Ecpm){
                    top2Ecpm = top1Ecpm;
                    top1Ecpm = ecpm;
                    top2EcpmOfferPrice = top1EcpmOfferPrice;
                    top1EcpmOfferPrice = offerprice;
                }
            }
            sortSolutionScore(solIdx,solScore,solCnt);
            //按排序概率展示
            int rankIdx[4];
            int rankWeight[3]={80,96,100};
            int actualRank = 0;
            double lastScore = -1;
            for(int i=0;i<solCnt && actualRank<4;i++){
                double s = solScore[solIdx[i]];
                if(s>lastScore){
                    rankIdx[actualRank++]=i;
                    lastScore = s;
                }
            }
            for(int i=actualRank;i<4;i++){
                rankIdx[i] = solCnt;
            }
            actualRank = actualRank>=4?3:actualRank;
            int totalWeight = rankWeight[actualRank-1];
            int randnum = std::abs(rng::randomInt())%totalWeight;
            int solutionRank = 0;
            for(;randnum>rankWeight[solutionRank];solutionRank++);

            int finalSolutionIdx =rankIdx[solutionRank]+std::abs(rng::randomInt())%(rankIdx[solutionRank+1]-rankIdx[solutionRank]);
            selectResult.finalSolution = solutions[solIdx[finalSolutionIdx]];
            rapidjson::Value& finalSolution = *(selectResult.finalSolution);
            rapidjson::Value& banner = bestBannerFromBannerGroup(bannerMap,finalSolution["bgid"].GetInt());
            selectResult.banner = &banner;
            //筛选出最后结果
            if(!banner.HasMember(ES_BANNER_FILED_JSON)){ //SSP没刷出广告,属于错误的情况
                return false;
            }
            selectResult.adplace = &adplace;
            double offerPrice = finalOfferPrice[solIdx[finalSolutionIdx]];
            int finalPriceType = finalSolution["pricetype"].GetInt();
            double basePrice = padplace!=NULL?adplace["baseprice"].GetDouble():0.0;
            selectResult.bidPrice = (int)calcBidPrice(finalPriceType,top1EcpmOfferPrice,top2EcpmOfferPrice,basePrice,offerPrice,fromSSP);
            selectResult.esResp = &esResp;
            return true;
        }

        bool AdSelectLogic::selectByPid(int seqId,const std::string& queryPid,bool isAdxPid,bool fromSSP){
            AdSelectCondition condition;
            if(isAdxPid){
                condition.adxpid = queryPid;
            }else{
                condition.mttyPid = queryPid;
            }
            return selectByCondition(seqId,condition,isAdxPid,fromSSP);
        }

    }
}