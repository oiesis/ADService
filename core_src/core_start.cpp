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
#include "muduo/base/Logging.h"
#include "core.h"

adservice::click::ClickModule g_clickService = nullptr;

namespace adservice {
    namespace server {

        volatile int ADService::instanceCnt = 0;
        ADServicePtr ADService::instance = nullptr;

        /**
         * 处理子进程结束信号
         */
        void handle_sigchild(int sig){
            int wstat;
            pid_t	pid;
            ADServicePtr service = ADService::getInstance();
            DebugMessage("in pid ",getpid()," handle sigchild");
	        while (true) {
                pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL );
                if (pid == 0)
                    return;
                else if (pid == -1)
                    return;
                else {
                     if(WEXITSTATUS(wstat)!=MTTY_EXIT_SUCCESS) { //并非正常退出
                            service->reLaunchModule(pid);
                     }
                }
            }
        }

        /**
         * 当外部kill进程时,进行处理
         */
        void handle_sigterm(int sig){
            DebugMessage("in pid: ",getpid()," handle signal ",sig);
	        ADServicePtr service = ADService::getInstance();
            if(service.use_count()>0)
                service->stop();
            if(g_clickService.use_count()>0){
		        DebugMessage("in pid: ",getpid()," terminate submodule click");
                g_clickService->stop();
            }
	        DebugMessage("in pid: ",getpid()," end of handle signal ",sig);
        }

        /**
         * 屏蔽不关心的信号
         */
        void signal_ignore() {
            struct sigaction sa;
            sa.sa_handler = SIG_IGN;
            sigaction(SIGPIPE, &sa, 0);
        }

        void signal_child(){
            struct sigaction sa;
            sa.sa_handler = handle_sigchild;
            sigaction(SIGCHLD,&sa,0);
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
            signal_child();
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
            switch(config.loggingLevel){
                case 1:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::DEBUG);
                    break;
                case 2:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::INFO);
                    break;
                case 3:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::WARN);
                    break;
                case 4:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::ERROR);
                    break;
                case 5:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::FATAL);
                    break;
                default:
                    break;
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
                switch(mt) {
                    case MODULE_TYPE::MODULE_CLICK:
                        g_clickService = std::make_shared<click::ClickService>(int(config.clickPort),
                                                                               int(config.clickThreads),
                                                                               bool(config.clickLogRemote),
                                                                               int(config.clickLoggerThreads));
                        g_clickService->start();
                        g_clickService.reset();
                        break;
                }
                exit(MTTY_EXIT_SUCCESS);
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
            //检查各模块是否需要被加载
            if(config.runClick) {
                DebugMessage("start click module");
		        launchModule(MODULE_TYPE::MODULE_CLICK);
            }
            while(running) {
                sleep(60);
                //do some monitor job
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

