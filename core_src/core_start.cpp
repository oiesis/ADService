//
// Created by guoze.lin on 16/1/20.
//

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <sys/wait.h>
#include "core.h"

adservice::corelogic::CoreModule g_coreService = nullptr;

namespace adservice {
    namespace server {

        volatile int ADService::instanceCnt = 0;
        ADServicePtr ADService::instance = nullptr;


        /**
         * 当外部kill进程时,进行处理
         */
        void handle_sigterm(int sig){
            DebugMessageWithTime("in pid: ",getpid()," handle signal ",sig);
	        ADServicePtr service = ADService::getInstance();
            if(service.use_count()>0)
                service->stop();
            if(g_coreService.use_count() > 0){
		        DebugMessage("in pid: ",getpid()," terminate submodule click");
                g_coreService->stop();
            }
	        DebugMessageWithTime("in pid: ",getpid()," end of handle signal ",sig);
        }

        /**
         * 屏蔽不关心的信号
         */
        void signal_ignore() {
            struct sigaction sa;
            sa.sa_handler = SIG_IGN;
            sigaction(SIGPIPE, &sa, 0);
        }

        void signal_kill(){
            struct sigaction sa;
            sa.sa_handler = handle_sigterm;
            sigaction(SIGTERM,&sa,0);
            sigaction(SIGINT,&sa,0);
	        sigaction(SIGHUP,&sa,0);
        }
        /**
         * 进行相关信号的注册
         */
        void ADService::dosignals() {
            signal_ignore();
            signal_kill();
        }

        /**
         * 重新启动模块
         */
        void ADService::reLaunchModule(pid_t pid) {
            MODULE_TYPE moduleType = moduleTypeOfPid(pid);
            if(moduleType!=MODULE_TYPE::MODULE_NON) {
                launchModule(moduleType);
            }
        }

        /**
         * 根据模块pid找到模块Id
         */
        MODULE_TYPE ADService::moduleTypeOfPid(pid_t pid){
            for(int i=MODULE_TYPE::MODULE_FIRST;i<=MODULE_TYPE::MODULE_LAST;i++){
                if(modules[i] == pid){
                    return MODULE_TYPE(i);
                }
            }
            return MODULE_TYPE::MODULE_NON;
        }

        /**
         * 进行服务初始化相关工作
         */
        void ADService::adservice_init() {
            memset(modules,0,sizeof(modules));
            dosignals();
	        int pid = write_pid(DEFAULT_DAEMON_FILE);
	        if(pid==0){
	   	        DebugMessage("can not write pid file.exit");
		        exit(0);
	        }
        }


        /**
         * 启动模块
         */
        void ADService::launchModule(MODULE_TYPE mt) {
            pid_t pid;
            if((pid =fork())<0){
                std::cerr<<" error when create new module!"<<std::endl;
                exit(1);
            }else if(pid == 0){ // submodule
                int result = 0;
                switch(mt) {
                    case MODULE_TYPE::MODULE_LOGIC:
                        g_coreService = std::make_shared<corelogic::CoreService>();
                        g_coreService->start();
                        result = g_coreService->isNeedRestart()?0:MTTY_EXIT_SUCCESS;
                        g_coreService.reset();
                        break;
                }
                exit(result);
            }else{
                modules[mt] = pid;
            }
        }

        /**
         * 开始服务
         */
        void ADService::adservice_start() {
            //开始服务的独立会话
            setsid();
            launchModule(MODULE_TYPE::MODULE_LOGIC);
            //todo: open a thread do monitor job
            //wait for child
            while(running) {
                int childStatus;
                int childPid = wait(&childStatus);
                if(childPid==-1){
                    return;
                }else if(childPid>0){
                    if(WEXITSTATUS(childStatus)!=MTTY_EXIT_SUCCESS){
                        reLaunchModule(childPid);
                    }
                }
                sleep(2);
            }
        }

        /**
         * 服务退出
         */
        void ADService::adservice_exit() {
        	for(int i=MODULE_TYPE::MODULE_FIRST;i<=MODULE_TYPE::MODULE_LAST;i++){
			    if(modules[i]!=0){
				    DebugMessage("main module exit,try to kill sub process ",modules[i]);
				    kill(modules[i],SIGTERM);
			    }
		    }
		    unlink(DEFAULT_DAEMON_FILE);
	    }

    }
}

