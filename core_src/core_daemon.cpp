//
// Created by guoze.lin on 16/2/1.
//

#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <cstdlib>
#include <cstdio>
#include "core.h"

namespace adservice {
    namespace server {
        /**
         * 读取pid文件,检查是否存在守护进程
         * @return : 如果存在返回进程ID,否则返回0
         */
        int check_pid(const char *pidfile) {
            int pid = 0;
            FILE *f = fopen(pidfile, "r");
            //文件不存在
            if (f == NULL)
                return 0;
            int n = fscanf(f, "%d", &pid);
            fclose(f);

            if (n != 1 || pid == 0 || pid == getpid()) {
                return 0;
            }
            //一个被占用的进程号,但进程不存在
            if (kill(pid, 0) && errno == ESRCH)
                return 0;
            //否则进程存在
            return pid;
        }

        /**
         * 写pid文件并加锁直到服务结束
         * @return :如果成功返回进程id,失败返回0
         */
        int write_pid(const char *pidfile) {
            FILE *f;
            int pid = 0;
            int fd = open(pidfile, O_RDWR | O_CREAT, 0644);
            if (fd == -1) {
                fprintf(stderr, "Can't create %s.\n", pidfile);
                return 0;
            }
            f = fdopen(fd, "r+");
            if (f == NULL) {
                fprintf(stderr, "Can't open %s.\n", pidfile);
                return 0;
            }

            if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
                int n = fscanf(f, "%d", &pid);
                fclose(f);
                if (n != 1) {
                    fprintf(stderr, "Can't lock and read pidfile.\n");
                } else {
                    fprintf(stderr, "Can't lock pidfile, lock is held by pid %d.\n", pid);
                }
                return 0;
            }

            pid = getpid();
            if (!fprintf(f, "%d\n", pid)) {
                fprintf(stderr, "Can't write pid.\n");
                close(fd);
                return 0;
            }
            fflush(f);

            return pid;
        }


        /**
         * 开启守护进程
         * @return :true 成功, false 失败
         */
        bool daemon_init(const char *pidfile) {
            int pid = check_pid(pidfile);

            if (pid) {
                fprintf(stderr, "ADCore is already running, pid = %d.\n", pid);
                return false;
            }

#ifdef __APPLE__
            fprintf(stderr, "'daemon' is deprecated: first deprecated in OS X 10.5 , use launchd instead.\n");
#else
            if (daemon(1,0)) {
                fprintf(stderr, "Can't daemonize.\n");
                return false;
            }
#endif

            pid = write_pid(pidfile);
            if (pid == 0) {
                return false;
            }

            return true;
        }

        /**
         * 守护进程退出,清理
         */
        int daemon_exit(const char *pidfile) {
            return unlink(pidfile);
        }
    }
}