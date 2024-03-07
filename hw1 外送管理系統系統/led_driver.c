/*
******************************************************************************
* \file led_driver.c
* \details Simple GPIO driver explanation
* \author EmbeTronicX
* \Tested with Linux raspberrypi 6.1.57-v8+
******************************************************************************
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h>	   //GPIO

// LED is connected to this GPIO
#define GPIO_26 (26)
#define GPIO_19 (19)
#define GPIO_13 (13)
#define GPIO_5  (5)
#define GPIO_0  (0)
#define GPIO_11 (11)
#define GPIO_9  (9)
#define GPIO_10 (10)

dev_t dev = 0;
static struct class *dev_class;
static struct cdev led_cdev;

static int __init led_driver_init(void);
static void __exit led_driver_exit(void);

/*************** Driver functions **********************/
static int seg_open(struct inode *inode, struct file *file);
static int seg_release(struct inode *inode, struct file *file);
static ssize_t seg_read(struct file *filp,
						char __user *buf, size_t len, loff_t *off);
static ssize_t seg_write(struct file *filp,
						const char *buf, size_t len, loff_t *off);
/******************************************************/

// File operation structure
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.read = seg_read,
		.write = seg_write,
		.open = seg_open,
		.release = seg_release,
};

/*
** This function will be called when we open the Device file
*/
static int seg_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

/*
** This function will be called when we close the Device file
*/
static int seg_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t seg_read(struct file *filp,
						char __user *buf, size_t len, loff_t *off)
{
	uint8_t gpio_state = 0;

	// reading GPIO value
	gpio_state = gpio_get_value(GPIO_26);

	// write to user
	len = 1;
	if (copy_to_user(buf, &gpio_state, len) > 0)
	{
		pr_err("ERROR: Not all the bytes have been copied to user\n");
	}

	pr_info("Read function : GPIO_21 = %d \n", gpio_state);

	return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t seg_write(struct file *filp,
						const char __user *buf, size_t len, loff_t *off)
{
	uint8_t rec_buf[10] = {0};
	int num[9][8]={
        {0,0,0,0,0,0,0,0}, // 0
        {0,0,0,0,0,0,0,1}, // 1
        {0,0,0,0,0,0,1,1}, // 2
        {0,0,0,0,0,1,1,1}, // 3
        {0,0,0,0,1,1,1,1}, // 4
        {0,0,0,1,1,1,1,1}, // 5
        {0,0,1,1,1,1,1,1}, // 6
        {0,1,1,1,1,1,1,1}, // 7
        {1,1,1,1,1,1,1,1}, // 8
    };

	if (copy_from_user(rec_buf, buf, len) > 0)
	{
		pr_err("ERROR: Not all the bytes have been copied from user\n");
	}

	gpio_set_value(GPIO_26,num[rec_buf[0]][0]);
	pr_info("8: %d",num[rec_buf[0]][0]);
	gpio_set_value(GPIO_19,num[rec_buf[0]][1]);
	pr_info("7: %d",num[rec_buf[0]][1]);
	gpio_set_value(GPIO_13,num[rec_buf[0]][2]);
	pr_info("6: %d",num[rec_buf[0]][2]);
	gpio_set_value(GPIO_5,num[rec_buf[0]][3]);
	pr_info("5: %d",num[rec_buf[0]][3]);
	gpio_set_value(GPIO_0,num[rec_buf[0]][4]);
	pr_info("4: %d",num[rec_buf[0]][4]);
	gpio_set_value(GPIO_11,num[rec_buf[0]][5]);
	pr_info("3: %d",num[rec_buf[0]][5]);
	gpio_set_value(GPIO_9,num[rec_buf[0]][6]);
	pr_info("2: %d",num[rec_buf[0]][6]);
    gpio_set_value(GPIO_10,num[rec_buf[0]][7]);
	pr_info("1: %d",num[rec_buf[0]][7]);

	return len;
}

/*
** Module Init function
*/
static int __init led_driver_init(void)
{
	/*Allocating Major number*/
	if ((alloc_chrdev_region(&dev, 0, 1, "led_Dev")) < 0)
	{
		pr_err("Cannot allocate major number\n");
		goto r_unreg;
	}
	pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
	cdev_init(&led_cdev, &fops);

	/*Adding character device to the system*/
	if ((cdev_add(&led_cdev, dev, 1)) < 0)
	{
		pr_err("Cannot add the device to the system\n");
		goto r_del;
	}

	/*Creating struct class*/
	if ((dev_class = class_create(THIS_MODULE, "led_class")) == NULL)
	{
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}

	/*Creating device*/
	if ((device_create(dev_class, NULL, dev, NULL, "led_dev")) == NULL)
	{
		pr_err("Cannot create the Device \n");
		goto r_device;
	}

	// Checking the GPIO is valid or not
	if (gpio_is_valid(GPIO_26) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_26);
		goto r_device;
	}

	if (gpio_is_valid(GPIO_19) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_19);
		goto r_device;
	}

	if (gpio_is_valid(GPIO_13) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_13);
		goto r_device;
	}

	if (gpio_is_valid(GPIO_5) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_5);
		goto r_device;
	}

	if (gpio_is_valid(GPIO_0) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_0);
		goto r_device;
	}

	if (gpio_is_valid(GPIO_11) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_11);
		goto r_device;
	}

	if (gpio_is_valid(GPIO_9) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_9);
		goto r_device;
	}

    if (gpio_is_valid(GPIO_10) == false)
	{
		pr_err("GPIO %d is not valid\n", GPIO_10);
		goto r_device;
	}


	// Requesting the GPIO
	if (gpio_request(GPIO_26, "GPIO_26") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_26);
		goto r_gpio;
	}
	
	if (gpio_request(GPIO_19, "GPIO_19") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_19);
		goto r_gpio;
	}

	if (gpio_request(GPIO_13, "GPIO_13") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_13);
		goto r_gpio;
	}

	if (gpio_request(GPIO_5, "GPIO_5") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_5);
		goto r_gpio;
	}
	
	if (gpio_request(GPIO_0, "GPIO_0") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_0);
		goto r_gpio;
	}

	if (gpio_request(GPIO_11, "GPIO_11") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_11);
		goto r_gpio;
	}

	if (gpio_request(GPIO_9, "GPIO_9") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_9);
		goto r_gpio;
	}

	if (gpio_request(GPIO_10, "GPIO_10") < 0)
	{
		pr_err("ERROR: GPIO %d request\n", GPIO_10);
		goto r_gpio;
	}

	// configure the GPIO as output
	gpio_direction_output(GPIO_26, 0);
	gpio_direction_output(GPIO_19, 0);
	gpio_direction_output(GPIO_13, 0);
	gpio_direction_output(GPIO_5, 0);
	gpio_direction_output(GPIO_0, 0);
	gpio_direction_output(GPIO_11, 0);
    gpio_direction_output(GPIO_9, 0);
    gpio_direction_output(GPIO_10, 0);



	/* Using this call the GPIO 21 will be visible in /sys/class/gpio/
	** Now you can change the gpio values by using below commands also.
	** echo 1 > /sys/class/gpio/gpio21/value  (turn ON the LED)
	** echo 0 > /sys/class/gpio/gpio21/value  (turn OFF the LED)
	** cat /sys/class/gpio/gpio21/value  (read the value LED)
	**
	** the second argument prevents the direction from being changed.
	*/
	gpio_export(GPIO_26, false);
	gpio_export(GPIO_19, false);
	gpio_export(GPIO_13, false);
	gpio_export(GPIO_5, false);
	gpio_export(GPIO_0, false);
	gpio_export(GPIO_11, false);
	gpio_export(GPIO_9, false);
    gpio_export(GPIO_10, false);
	pr_info("Device Driver Insert...Done!!!\n");
	return 0;

r_gpio:
	gpio_free(GPIO_26);
	gpio_free(GPIO_19);
	gpio_free(GPIO_13);
	gpio_free(GPIO_5);
	gpio_free(GPIO_0);
	gpio_free(GPIO_11);
	gpio_free(GPIO_9);
    gpio_free(GPIO_10);
r_device:
	device_destroy(dev_class, dev);
r_class:
	class_destroy(dev_class);
r_del:
	cdev_del(&led_cdev);
r_unreg:
	unregister_chrdev_region(dev, 1);

	return -1;
}

/*
** Module exit function
*/
static void __exit led_driver_exit(void)
{
	gpio_unexport(GPIO_26);
	gpio_free(GPIO_26);
	gpio_unexport(GPIO_19);
	gpio_free(GPIO_19);
	gpio_unexport(GPIO_13);
	gpio_free(GPIO_13);
	gpio_unexport(GPIO_5);
	gpio_free(GPIO_5);
	gpio_unexport(GPIO_0);
	gpio_free(GPIO_0);
	gpio_unexport(GPIO_11);
	gpio_free(GPIO_11);
	gpio_unexport(GPIO_9);
	gpio_free(GPIO_9);
    gpio_unexport(GPIO_10);
	gpio_free(GPIO_10);
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&led_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove...Done!!\n");
}

module_init(led_driver_init);
module_exit(led_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
MODULE_VERSION("1.32");