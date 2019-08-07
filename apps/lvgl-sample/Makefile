RTEMS_ROOT ?= $(RTEMS_PREFIX)
RTEMS_BSP ?= beagleboneblack
export LVGL_DIR=$(PWD)

IMAGE_CREATOR ?= $(PWD)/../../build/create-sdcardimage.sh

include $(PWD)/../../install/rtems/5/make/custom/$(RTEMS_BSP).mk
include $(PWD)/lvgl/lvgl.mk

CFLAGS += -O0
#LDFLAGS +=

LVGL = $(BUILDDIR)/liblvgl.a
LVGL_OBJS = $(CSRCS:%.c=$(BUILDDIR)/%.o)
LVGL_DEPS = $(CSRCS:%.c=$(BUILDDIR)/%.d)
APP = $(BUILDDIR)/lvgl-sample
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe $(APP)_sdcard.img $(LVGL) $(LVGL_OBJS)


$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS) $(LVGL)
	$(CCLINK) $^ -lftpd -lbsd -lm -lz -llvgl -L $(BUILDDIR) -o $@

$(LVGL): $(LVGL_OBJS)
	ar rcs $(LVGL) $(LVGL_OBJS)

#$(LVGL_OBJS): $(CSRCS)

$(APP)_sdcard.img: $(APP).exe
	./create_dtb.sh 
	$(IMAGE_CREATOR) $< $@

clean:
	rm -rf $(BUILDDIR) tmp_sdcard_dir.*

debugoutput:
	echo $(PATH)

-include $(APP_DEPS)
-include $(LVGL_DEPS)
