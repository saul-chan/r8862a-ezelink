bin_PROGRAMS = cwmpd

cwmpd_SOURCES = conf.c  cwmpd.c  httpd.c  process.c  thread.c agent.c ubus.c modules/data_model.c modules/data_model_util.c


AM_CPPFLAGS = -DUSE_CWMP_MEMORY_POOL -I$(top_srcdir)/include  -I$(top_srcdir)/cwmpd/include -I$(top_srcdir)/libpool/include  -I$(top_srcdir)/libxmlet/include -I$(top_srcdir)/libstun/include -I$(top_srcdir)/libcwmp/include -I$(STAGING_DIR)/usr/include



cwmpd_LDADD =  -L$(top_srcdir)/libpool/src -L$(top_srcdir)/libxmlet/src -L$(top_srcdir)/libstun/src -L$(top_srcdir)/libcwmp/src -lpool -lxmlet -lstun -lcwmp  -lpthread -lubox -lubus -luci -lclsapi_wifi -lclsapi_base -lclsapi_net -lnl-tiny -liwinfo


cwmpd_CPPFLAGS = $(AM_CPPFLAGS) 

cwmpd_LDFLAGS =  -D_GNU_SOURCE -DUSE_CWMP_MEMORY_POOL  
