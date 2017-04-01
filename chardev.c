// we create a read-only char device that tells us w many times we hve read from the dev file
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<acm/uaccess.h> //This is for put_user which is supposed to incrememnt the count
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
// whatever structure was not declared would be considered to be NULL
#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices
proc/devices displays the name of all devices that are on the system */
#define BUF_LEN 80		/* Max length of the message from the device */
/* Global variables are declared as static so that they are global within the file*/
static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

/* the driver structire goes here*/
static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
}
int init_module(void)
{
  Major = register_chrdev(0, DEVICE_NAME, &fops); //Here 0 is the unsigned major int
  // we pass 0 because when we do so the kernel dynamically allocates a memory
  if(Major<0) //it means that the registration has failed
  {
    printk(KERN_ALERT "Registring char device failed with %d\n", Major);
    return Major
  }
  printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

	return SUCCESS;
}
void cleanup_module(void)
{
  int ret = unregister_chrdev(Major, DEVICE_NAME);
  if(ret<0)
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}
/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE); //should increment the counter

	return SUCCESS;
}
