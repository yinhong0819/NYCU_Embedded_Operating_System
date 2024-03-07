obj-m := mydev.o
KDIR = /home/angus/linux

all: RPI_build

clean: x86_clean RPI_clean

x86_build: mydev.c
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

x86_clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

RPI_build: mydev.c
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules

RPI_clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean