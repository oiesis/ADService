//
// Created by guoze.lin on 16/2/14.
//

#include <sched.h>
#include "core_executor.h"
#include "common/spinlock.h"

namespace adservice{
    namespace server{

        struct RunInCore{
            RunInCore(){
                runCore = 0;
                SPIN_INIT(this);
            }
            operator()(){
                int myCore = 0;
                SPIN_LOCK(this);
                myCore = runCore++;
                SPIN_UNLOCK(this);
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(myCore,&cpuset);
                pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&cpuset);
            }
            ~RunInCore(){
                SPIN_DESTROY(this);
            }
            volatile int runCore;
            volatile spinlock lock;
        };

        void Executor::start(){
            if(pureCompute){
                configureForCompute();
                threadpool.setThreadInitCallback(std::bind(RunInCore()));
            }
            threadpool.start(threadNum);
        }

        void Executor::configureForCompute() {
            long nprocessors = sysconf(_SC_NPROCESSORS_ONLN);
            if(nprocessors<=0){
                threadNum = DEFAULT_THREAD_NUM;
            }else{
                threadNum = nprocessors;
            }
        }

    }
}