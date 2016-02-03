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

namespace adservice {
    namespace server {

        //维护一个点击模块的弱引用
        static click::ClickModule_weak w_clickModule = nullptr;

        /**
         * 处理子进程结束信号
         */
        void handle_sigchild(){
            int wstat;
            pid_t	pid;
            ADServicePtr service = ADService::getInstance();
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
        void handle_sigkill(){
            ADServicePtr service = ADService::getInstance();
            service->stop();
            if(w_clickModule!=nullptr){
                w_clickModule->stop();
            }
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
            sa.sa_handler = handle_sigkill;
            sigaction(SIGKILL,&sa,0);
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
                        click::ClickModule clickService = std::make_shared(int(config.clickPort), int(config.clickThreads));
                        w_clickModule = clickService;
                        clickService->start();
                        w_clickModule = nullptr;
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
            if(config.runClick) {
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
        }

    }
}

