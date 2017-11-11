#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/mempolicy.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malware Team");
MODULE_DESCRIPTION("A rootkit");
MODULE_VERSION("1.0.0");
MODULE_SUPPORTED_DEVICE("None");

struct task_struct *task;

static int __init rootkit_init(void){
	printk(KERN_INFO "Hello World!");

	for_each_process(task){
		printk(KERN_INFO "Task id: %d", task->pid);
		printk(KERN_INFO "Task UID: %d, Task GID: %d", task->loginuid.val, task->tgid);
		printk(KERN_INFO "Task mempolicy: %d", task->mempolicy->mode);
	}

	return 0;
}

static void __exit rootkit_exit(void){
	printk(KERN_INFO "Goodbye World!");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
