//
// Created by guoze.lin on 16/1/20.
//

#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <signal.h>
#include "core.h"

namespace adservice {
    namespace server {

        /**
         * 屏蔽不关心的信号
         */
        void signal_ignore() {
            struct sigaction sa;
            sa.sa_handler = SIG_IGN;
            sigaction(SIGPIPE, &sa, 0);
        }

        /**
         * 进行相关信号的注册
         */
        void ADService::dosignals() {
            signal_ignore();
        }

        /**
         * 进行服务初始化相关工作
         */
        void ADService::adservice_init() {

        }

        /**
         * 开始服务
         */
        void ADService::adservice_start() {
            if (daemonFile!=NULL) {
                if (!daemon_init(daemonFile)) {
                    exit(1);
                }
            }
        }

        /**
         * 服务退出
         */
        void ADService::adservice_exit() {
        }
    }
}

