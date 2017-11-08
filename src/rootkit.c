#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/fs.h>

static int __init rootkit_init(void){;}
static void __exit rootkit_exit(void){;}

module_init(rootkit_init);
module_exit(rootkit_exit);