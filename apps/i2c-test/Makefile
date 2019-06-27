RTEMS_ROOT ?= $(RTEMS_PREFIX)
RTEMS_BSP ?= beagleboneblack


include $(PWD)/../../install/rtems/5/make/custom/$(RTEMS_BSP).mk
IMAGE_CREATOR ?= $(PWD)/../../build/create-sdcardimage.sh

CFLAGS += -O0

APP = $(BUILDDIR)/app_i2c_test
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe $(APP)_sdcard.img

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -o $@

$(APP)_sdcard.img: $(APP).exe
	$(IMAGE_CREATOR) $< $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
