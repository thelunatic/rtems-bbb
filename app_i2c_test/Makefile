RTEMS_ROOT ?= $(PWD)/../rtems-install/rtems-4.12
RTEMS_BSP ?= beagleboneblack

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

CFLAGS += -O0

APP = $(BUILDDIR)/app_i2c_test
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -o $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
