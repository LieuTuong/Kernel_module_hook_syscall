#include<linux/init.h>
#include<linux/module.h>
#include<linux/random.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#define DRIVER_DESC "Do an 2 he dieu hanh"
#define DRIVER_AUTHOR "Lieu Tuong"
#define DRIVER_NAME "mdriver"



struct _vchr_driver
{	
	dev_t dev_num;

	struct class *dev_class;
	struct device *dev;
	struct cdev *mcdev;
}vchr_driver;


int intToStr(uint32_t num, char numStr[11])
{
	int i = 0;
	while(num!=0)
	{
		numStr[i] = (num % 10) + '0';
		num = num / 10;
		i++;
	}
	numStr[i]='\0';
	return i;
	
}

static int device_open (struct inode *inode, struct file *filep)
{
	printk(KERN_INFO "mdriver is opened\n");
	return 0;
}

static int device_close (struct inode *inode, struct file *filep)
{
	printk(KERN_INFO "mdriver is closed\n");
	return 0;
}

static ssize_t device_write(struct file *filep, const char *buff, size_t len, loff_t *offset)
{
	printk(KERN_INFO "Sorry, mdriver is read only\n");
	return 0;
}

static ssize_t device_read(struct file *filep, char __user *userbuf, size_t len, loff_t *offset)
{
	int errors = 0;
	uint32_t num;
	uint32_t numLen;
	char numStr[11];
	get_random_bytes(&num, sizeof(num));
	numLen = intToStr(num, numStr);
	
	errors =copy_to_user(userbuf,numStr,numLen);
	return errors == 0 ? numLen : -EFAULT;
	
}

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read,
};



static int __init mdriver_init(void)
{
	int ret = 0;
	//cap phat dong device number
	vchr_driver.dev_num = 0;
	ret = alloc_chrdev_region(&vchr_driver.dev_num, 0, 1, DRIVER_NAME);
	if (ret < 0 )
	{
		printk("alloc_chrdev_region failed: can't register device number dynamically\n");
		goto failed_register_devnum;
	}
	
	
	//tao device file
	vchr_driver.dev_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (vchr_driver.dev_class == NULL)
	{
		printk("class_create failed\n");
		goto failed_create_class;
	}
	
	vchr_driver.dev = device_create(vchr_driver.dev_class, NULL, vchr_driver.dev_num,NULL,DRIVER_NAME);
	if (IS_ERR(vchr_driver.dev))
	{
		printk("device_create failed\n");
		goto failed_create_device;
	}


	// dang ki cac entry point
	vchr_driver.mcdev = cdev_alloc();
	cdev_init(vchr_driver.mcdev, &fops);
	if (cdev_add(vchr_driver.mcdev,vchr_driver.dev_num, 1) < 0)
	{
		printk("failed to add a char device to the system\n");
		goto failed_create_entry;
	}

	printk("loaded mdriver succesfully\n");

	return 0;

failed_create_entry:
	cdev_del(vchr_driver.mcdev);
failed_create_device:
	class_destroy(vchr_driver.dev_class);
failed_create_class:
	unregister_chrdev_region(vchr_driver.dev_num,1);
failed_register_devnum:
	return ret;
}


static void __exit mdriver_exit(void)
{
	cdev_del(vchr_driver.mcdev);
	device_destroy(vchr_driver.dev_class, vchr_driver.dev_num);
	class_destroy(vchr_driver.dev_class);
	unregister_chrdev_region(vchr_driver.dev_num, 1);
	printk("removed mdriver driver\n");
}



module_init(mdriver_init);
module_exit(mdriver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR(DRIVER_AUTHOR);
