<<<<<<< HEAD
obj-m += keylogger.o
obj-m += reverseTCP.o
=======
obj-m += keylogger.o reverseTCP.o
>>>>>>> f1318fde5e6a5a1b1074dc87e94a58db3766bdc6

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
