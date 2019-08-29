RTEMS_ROOT ?= $(RTEMS_PREFIX)
RTEMS_BSP ?= beagleboneblack
export LVGL_DIR=$(PWD)

IMAGE_CREATOR ?= $(PWD)/../../build/create-sdcardimage.sh

include $(PWD)/../../install/rtems/5/make/custom/$(RTEMS_BSP).mk

CFLAGS += -O0
#LDFLAGS +=

APP = $(BUILDDIR)/lvgl-waf-sample
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

LV_INCLUDES = "-I/home/lunatic/littlevgl-rsb/"
LV_INCLUDES += "-I/home/lunatic/littlevgl-rsb/lvgl"
LV_INCLUDES += "-I/home/lunatic/littlevgl-rsb/lv_drivers/display"

all: $(BUILDDIR) $(APP).exe $(APP)_sdcard.img


$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -lftpd -lbsd -lm -lz -llvgl -o $@

$(APP)_sdcard.img: $(APP).exe
	./create_dtb.sh 
	$(IMAGE_CREATOR) $< $@

$(APP_OBJS): $(APP_PIECES)
	$(CCLINK) -c $< $(LV_INCLUDES) -o $@

clean:
	rm -rf $(BUILDDIR) tmp_sdcard_dir.*

debugoutput:
	echo $(PATH)

-include $(APP_DEPS)
