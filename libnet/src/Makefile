include $(TOPDIR)/config.mk

src-y += AdTcp.cpp
src-y += AdTcpServer.cpp
src-y += AdSession.cpp
src-y += AdServer.cpp
src-y += AdClient.cpp
src-y += AdMult.cpp
src-y += AdUdp.cpp
src-y += AdNetBase.cpp
src-y += AdEvent.cpp

static-lib-y	+= $(TOPDIR)/libextern/lib/libevent-2.0.so.5

lib-m = libnet.so
include $(TOPDIR)/build.mk
