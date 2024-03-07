#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
MODULE_LICENSE("GPL");

// File Operations
char name_buf;

static ssize_t my_read(struct file *fp, char *buf, size_t count, loff_t *fpos) 
{
    printk("call read\n");

    char *seg_for_c[27] = {
        "1111001100010001", // A
        "0000011100000101", // b
        "1100111100000000", // C
        "0000011001000101", // d
        "1000011100000001", // E
        "1000001100000001", // F
        "1001111100010000", // G
        "0011001100010001", // H
        "1100110001000100", // I
        "1100010001000100", // J
        "0000000001101100", // K
        "0000111100000000", // L
        "0011001110100000", // M
        "0011001110001000", // N
        "1111111100000000", // O
        "1000001101000001", // P
        "0111000001010000", //q
        "1110001100011001", //R
        "1101110100010001", //S
        "1100000001000100", //T
        "0011111100000000", //U
        "0000001100100010", //V
        "0011001100001010", //W
        "0000000010101010", //X
        "0000000010100100", //Y
        "1100110000100010", //Z
        "0000000000000000"
    };

    int num = 0;

    if(name_buf >= 'A' && name_buf <= 'Z')
        num = name_buf - 'A';
    else if(name_buf >= 'a' && name_buf <= 'z')
        num = name_buf -'a';
    pr_info("num is : %d\n",num);


    if( copy_to_user(buf, seg_for_c[num], 16) > 0) 
    {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }
    else
    {
        printk("Copy to user = %c \n", name_buf);
        printk("seg_for_c[%d]: %s",num,seg_for_c[num]);
    }

   
    return count;
}


static ssize_t my_write(struct file *fp,const char *buf, size_t count, loff_t *fpos)
{
    printk("call write\n");
    
    if( copy_from_user(&name_buf,buf,count) > 0)
    {
        pr_err("Error: Not all the bytes have been copied to user\n");
    }
    pr_info("Write from user : %c\n",name_buf);

    return count;
}

static int my_open(struct inode *inode, struct file *fp) 
{
    printk("call open\n");
    return 0;
}

struct file_operations my_fops = {
    read: my_read,
    write: my_write,
    open: my_open
};

#define MAJOR_NUM 255
#define DEVICE_NAME "my_dev"

static int my_init(void) {
    printk("call init\n");
    if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops) < 0) {
        printk("Can not get major %d\n", MAJOR_NUM);
        return (-EBUSY);
    }
    printk("My device is started and the major is %d\n", MAJOR_NUM);
    return 0;
}

static void my_exit(void) {
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk("call exit\n");
}


module_init(my_init);
module_exit(my_exit);