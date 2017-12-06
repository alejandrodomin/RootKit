#include "tester.h"   

int main(){
   int dev_ret, fd;
   char message_string[BUFFER_SIZE];
   int busy = 1;
   
   
   int count = 0;//wait 20 second for devise, exit if not available after 20 sec
   do
   {
        fd = open("/dev/testchar", O_RDWR);// open read/write access
        if (fd < 0){
            puts("device not open...Error");
            puts("Check that device is listed on /dev directory or the user priviledges\n");
            return -1;
        }
        
        //check if device not locked or no other process is using
        busy = ioctl(fd,DEVICE_IN_USE_IOCTL,NULL) & ioctl(fd,NUMBER_OF_DEVICES_IOCTL,NULL);
        if(busy){
            if(close(fd) < 0){
                puts("device not closed...Error");
                return -1;
            }
        }
        
        if(!count && busy)
            printf("===========> waiting on devise <============\n");

        count++;
        usleep(WAITTIME);     
   }
   while(busy && (count < MAXWAIT));
   
   if(busy){//close program
       puts("device busy for too long...exiting\n");
       return -1;
   }
   
   puts("\nEnter string to send to device [max 255 characters]:");
   scanf("%[^\n]%*c", message_string);//ignore white spaces             
   puts("Writing to device...");
   
   dev_ret = write(fd, message_string, strlen(message_string)); 
   if (dev_ret < 0)
   {
      puts("Failed to write to device.\n");
      return -1;
   }

   puts("Press ENTER to read back from device...\n");
   char enter = 0;
   while(enter != '\n')
   {
        enter = getchar();//block
   }

   puts("Reading from device...");
   dev_ret = read(fd, buffer, BUFFER_SIZE);        
   if (dev_ret < 0){
      puts("Failed to read from device.\n");
      return -1;
   }
   printf("The message is: %s\n", buffer);
   
   if(close(fd) < 0){
        puts("device not closed...Error");
        return -1;
   }
   puts("device closed");
   return 0;
}
