RTEMS_ROOT ?= $(RTEMS_PREFIX)
RTEMS_BSP ?= beagleboneblack

IMAGE_CREATOR ?= $(PWD)/../../build/create-sdcardimage.sh

include $(PWD)/../../install/rtems/5/make/custom/$(RTEMS_BSP).mk

CFLAGS += -O2
#LDFLAGS +=

APP = $(BUILDDIR)/fb-sample
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe $(APP)_sdcard.img

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -lftpd -lbsd -lm -lz -o $@

$(APP)_sdcard.img: $(APP).exe
	./create_dtb.sh 
	$(IMAGE_CREATOR) $< $@

clean:
	rm -rf $(BUILDDIR) tmp_sdcard_dir.*

debugoutput:
	echo $(PATH)

-include $(APP_DEPS)
