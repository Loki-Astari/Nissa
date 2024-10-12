THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= Nissa.prog

LDLIBS						+= -levent -levent_pthreads
LINK_LIBS					+= ThorsSocket ThorsLogging
FILE_WARNING_FLAGS			+= -Wno-missing-braces -Wno-unused-parameter

include $(THORSANVIL_ROOT)/build/tools/Makefile
