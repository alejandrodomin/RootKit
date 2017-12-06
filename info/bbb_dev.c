#include "bbb_dev.h"

module_param(name, charp, S_IRUGO);

MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");  ///< parameter descript



static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = device_open,
   .write = device_write,
   .release = device_release
};

static int __init bbb_dev_init(void) {
	//save major number
   major = register_chrdev(0, DEVICE_NAME, &fops);
   if (major<0){
      printk(KERN_ALERT "bbb_dev failed to register major number\n");
      return major;
   }
   printk(KERN_INFO "bbb_dev: registered a major number %d\n", major);

   // Register the device class
   bbb_devClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(bbb_devClass)) {
      unregister_chrdev(major, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class. Exiting init\n");
      return PTR_ERR(bbb_devClass);
   }
   printk(KERN_INFO "bbb_dev: device class registered correctly\n");

   // Register the device driver
   bbb_devDevice = device_create(bbb_devClass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
   if (IS_ERR(bbb_devDevice)) {     //unregister if error
      class_destroy(bbb_devClass);
      unregister_chrdev(major, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device class\n");
      return PTR_ERR(bbb_devDevice);
   }
   printk(KERN_INFO "bbb_dev: device class created correctly\n");

   mutex_init(&bbb_devMutex);

   printk(KERN_INFO "TDR: Hello %s from LED init!\n", name);
   gpio_addr = ioremap(GPIO1_START_ADDR, GPIO1_SIZE);
   if(!gpio_addr) {
     printk (KERN_ERR "ERROR: Failed to remap memory for GPIO Bank 1.\n");
   }

   gpio_setdataout_addr   = gpio_addr + GPIO_SETDATAOUT;
   gpio_cleardataout_addr = gpio_addr + GPIO_CLEARDATAOUT;

   BBBremoveTrigger(); //remove trigger when device driver init

   return 0;
}

static int device_open(struct inode *inodep, struct file *filep){

   if (!mutex_trylock(&bbb_devMutex)) {
      //print this to user space, not kernel log
      printk("Cannot Access the Device Driver bbb_dev because another process is using it.\n");
      //return something here
      return -EBUSY;
   }

   printk(KERN_INFO "bbb_dev: Device has been successfully opened.\n");
   return 0;
}

//read from the user and WRITE to the beaglebone at the same time
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {

   //send the message to the B^3 to be displayed using the LEDs
   int i=0; char *letter; char* space = " ";

   while(buffer[i]!='\0') {

    if(!(buffer[i] == space[0])) {
      letter = mcodestring(buffer[i]);

      int j=0;
      while(letter[j]!='\0') {
        switch(letter[0]) {

            case '.':
                BBBledOn();
                msleep(TIMING);

            case '-':
                BBBledOn();
                msleep(TIMING * 3);

            default:
                BBBledOff();
                msleep(TIMING);

         }
         BBBledOff();
         msleep(TIMING);
        j++;
      }
       //inter-letter spacing
        BBBledOff();
        msleep(TIMING * 3); 
    }

     //inter-word spacing code
      else {
        BBBledOff();
        msleep(TIMING * 7);
      }

    i++;
   }
   return 0;
}


static int device_release(struct inode *inodep, struct file *filep) {

   mutex_unlock(&bbb_devMutex);

   printk(KERN_INFO "bbb_dev: Device has been successfully closed\n");
   return 0;
}

static void __exit bbb_dev_exit(void) {
   BBBledOff();
   BBBstartHeartbeat();
   printk(KERN_INFO "LED has been successfully restored to its previous condition");


   mutex_destroy(&bbb_devMutex);
   device_destroy(bbb_devClass, MKDEV(major, 0));
   class_unregister(bbb_devClass);
   class_destroy(bbb_devClass);
   unregister_chrdev(major, DEVICE_NAME);

   printk(KERN_INFO "bbb_dev: Goodbye from the LKM!\n");
}

module_init(bbb_dev_init);
module_exit(bbb_dev_exit);


//Pons Code
void BBBremoveTrigger(){

   int err = 0;

  strcpy(fullFileName, LED0_PATH);
  strcat(fullFileName, "/");
  strcat(fullFileName, "trigger");
  printk(KERN_INFO "File to Open: %s\n", fullFileName);
  filepath = fullFileName; // set for disk write code
  err = setup_disk();
  err = write_vaddr_disk("none", 4);
  cleanup_disk();
}

void BBBstartHeartbeat(){

     int err = 0;


  strcpy(fullFileName, LED0_PATH);
  strcat(fullFileName, "/");
  strcat(fullFileName, "trigger");
  printk(KERN_INFO "File to Open: %s\n", fullFileName);
  filepath = fullFileName; // set for disk write code
  err = setup_disk();
  err = write_vaddr_disk("heartbeat", 9);
  cleanup_disk();
}

void BBBledOn(){
*gpio_setdataout_addr = USR_LED;
}


void BBBledOff(){
*gpio_cleardataout_addr = USR_LED;
}


static void disable_dio() {
   dio = 0;
   reopen = 1;
   cleanup_disk();
   setup_disk();
}

int setup_disk() {
   mm_segment_t fs;
   int err;

   fs = get_fs();
   set_fs(KERNEL_DS);

   if (dio && reopen) {
      f = filp_open(filepath, O_WRONLY | O_CREAT | O_LARGEFILE | O_SYNC | O_DIRECT, 0444);
   } else if (dio) {
      f = filp_open(filepath, O_WRONLY | O_CREAT | O_LARGEFILE | O_TRUNC | O_SYNC | O_DIRECT, 0444);
   }

   if(!dio || (f == ERR_PTR(-EINVAL))) {
      f = filp_open(filepath, O_WRONLY | O_CREAT | O_LARGEFILE | O_TRUNC, 0444);
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

void cleanup_disk() {
   mm_segment_t fs;

   fs = get_fs();
   set_fs(KERNEL_DS);
   if(f) filp_close(f, NULL);
   set_fs(fs);
}

ssize_t write_vaddr_disk(void * v, size_t is) {
   mm_segment_t fs;

   ssize_t s;
   loff_t pos;

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

char *morse_code[40] = {"",
".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",
".-..","--","-.","---",".--.","--.-",".-.","...","-","..-","...-",
".--","-..-","-.--","--..","-----",".----","..---","...--","....-",
".....","-....","--...","---..","----.","--..--","-.-.-.","..--.."};



char * mcodestring(int asciicode) {
   char *mc;   

   if (asciicode > 122)  // Past 'z'
      mc = morse_code[CQ_DEFAULT];
   else if (asciicode > 96)  // Upper Case
      mc = morse_code[asciicode - 96];
   else if (asciicode > 90)  // uncoded punctuation
      mc = morse_code[CQ_DEFAULT];
   else if (asciicode > 64)  // Lower Case 
      mc = morse_code[asciicode - 64];
   else if (asciicode == 63)  // Question Mark
      mc = morse_code[39];    // 36 + 3 
   else if (asciicode > 57)  // uncoded punctuation
      mc = morse_code[CQ_DEFAULT];
   else if (asciicode > 47)  // Numeral
      mc = morse_code[asciicode - 21];  // 27 + (asciicode - 48) 
   else if (asciicode == 46)  // Period
      mc = morse_code[38];  // 36 + 2 
   else if (asciicode == 44)  // Comma
      mc = morse_code[37];   // 36 + 1
   else
      mc = morse_code[CQ_DEFAULT];
   return mc;
}
