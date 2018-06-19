ifneq ($(KERNELRELEASE),)
obj-m := zforceair_ts.o
else
#CROSS_COMPILE= $(HOME)/dragonboard/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/arm-linux-androideabi-
CROSS_COMPILE= $(HOME)/dragonboard/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
KDIR := $(HOME)/dragonboard/out/target/product/msm8909w/obj/KERNEL_OBJ/
all:
	$(MAKE) -C $(KDIR) M=$$PWD ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE)
endif
