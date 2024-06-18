KDIR = /lib/modules/$(shell uname -r)/build
PWD  = $(shell pwd)
MAKE = /usr/bin/make

%.ko : obj-m=$(*).o
%.ko : %.c
	$(MAKE) $@ obj-m=$(obj-m) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.order *.cmd *.symvers *.mod.c *.mod .o*


# ja dit is leuk
