#ifndef BBB_DEV_H
#define BBB_DEV_H


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


#define CQ_DEFAULT	0

#define  DEVICE_NAME "BBB_dev"
#define  CLASS_NAME  "BBB"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Victor Gonzalez");
MODULE_DESCRIPTION("Linux device driver for the B^3 in order to flash the LEDs in Morse Code");
MODULE_VERSION("1.0");

//define listing for the LEDs to turn on
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

//variables for the device driver
static int major = 0;
static struct class*  bbb_devClass  = NULL;
static struct device* bbb_devDevice = NULL;
static DEFINE_MUTEX(bbb_devMutex);  //mutex unlocked by default

//static variables declared for the LED mapping, notice they are volatile
static volatile void *gpio_addr;
static volatile unsigned int *gpio_setdataout_addr;
static volatile unsigned int *gpio_cleardataout_addr;


//file declarations for the device driver
static int device_open(struct inode *, struct file *);
//static ssize_t dev_read(struct file *, char *, size_t, loff_t *); //display to LEDs (Didn't need read)
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);  //get strings from the user to be displayed in Morse Code
static int device_release(struct inode *, struct file *);
//return something to the LEDs in order to display Morse code

//LED func and others
ssize_t write_vaddr_disk(void *, size_t);
int setup_disk(void);
void cleanup_disk(void);
static void disable_dio(void);

char * mcodestring(int asciicode);

static struct file * f = NULL;
static int reopen = 0;
static char *filepath = 0;
static char fullFileName[1024];
static int dio = 0;

static char *name = "world";




#endif
