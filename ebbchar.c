#include<linux/module.h>  //used to load LKM into the kernel
#include<linux/limit.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/fs.h>
#include<linux/init.h>  //Used for maekup functions
#include <asm/uaccess.h>  //required to copy to user function
#define  DEVICE_NAME "ebbchar";
#define CLASS_NAME "ebb";

static int majorNumber; //stores the device Number
static char msg[250] = {0}; //used to store message passed to userspace
static short size_of_message;//used to remember size of string
static int numberOpens = 0; //Number of times the device is opened is tracked
static struct class* ebbcharClass = NULL; //device drive class struct pointer
static struct device* ebbcharDevice = NULL; //device driver device struct pointer

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
  .open = dev_open,
  .write = dev_write,
  .read = dev_read,
  .release = dev_release,
};

/*  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */

 static int __init ebbchar_init(void)
 {
      printk(KERN_INFO "EBBChar: Initializing the EBBChar LKM\n");
      majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
      if(majorNumber<0)
      {
        printk(KERN_ALERT "EBBChar failed to register a major number\n");
        return 0;
      }
      printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", majorNumber);
      ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
      if (IS_ERR(ebbcharClass))
      {                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ebbcharClass);          // Correct way to return an error on a pointer
      }
      printk(KERN_INFO "EBBChar: device class registered correctly\n");


      ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
      if (IS_ERR(ebbcharDevice))
      {
        class_destroy(ebbcharClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ebbcharDevice);
      }
     printk(KERN_INFO "EBBChar: device class created correctly\n");
     static void __exit ebbchar_exit(void)
     {
       device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // remove the device
       class_unregister(ebbcharClass);                          // unregister the device class
       class_destroy(ebbcharClass);                             // remove the device class
       unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
       printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
     }
     static int dev_open(struct inode* struct file*)
     {
       numberOpens++;
       printk(KERN_INFO "EBBChar: Device has been opened %d time(s)\n", numberOpens);
       return 0;
     }
/** function when the device is being read from the user space
 *  In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  filep A pointer to a file object (defined in linux/fs.h)
 *  buffer The pointer to the buffer to which this function writes the data
 *  len The length of the b
 *  offset The offset if required
 */
 static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
 {
   int error_count = 0;
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0)
   {            // if true then have success
      printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else
   {
      printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
   return 0;
 }

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "EBBChar: Received %zu characters from the user\n", len);
   return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}

module_init(ebbchar_init);
module_exit(ebbchar_exit);
