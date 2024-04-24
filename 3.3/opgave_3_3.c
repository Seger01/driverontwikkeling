#include <linux/cdev.h> // Include for cdev related functions
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

// Define cdev structure
static struct cdev my_cdev;

static int hello_open(struct inode* inode, struct file* file) {
    printk(KERN_ALERT "hello_open()\n");
    return 0;
}

static int hello_release(struct inode* inode, struct file* file) {
    printk(KERN_ALERT "hello_release()\n");
    return 0;
}

static int hello_init(void) {
    printk(KERN_ALERT "Hello, world\n");
    //
    // int result;
    // dev_t dev = MKDEV(0, 0); // Device number
    //
    // // Allocate memory for cdev
    // cdev_init(&my_cdev, &fops);
    //
    // // Add cdev to the kernel
    // result = cdev_add(&my_cdev, dev, 1);
    // if (result < 0) {
    //     printk(KERN_ALERT "Failed to register cdev\n");
    //     return result;
    // }
    //
    // printk(KERN_ALERT "Registered character device\n");
    return 0;
}

static ssize_t hello_read(struct file* file, char __user* buf, size_t lbuf, loff_t* ppos) {
    printk(KERN_ALERT "hello_read()\n");
    return 0;
}

static ssize_t hello_write(struct file* file, const char __user* buf, size_t lbuf, loff_t* ppos) {
    printk(KERN_ALERT "hello_write()\n");
    return 0;
}

struct file_operations fops = {
    .read = hello_read,
    .write = hello_write,
    .open = hello_open,
    .release = hello_release,
};

static void hello_exit(void) {
    printk(KERN_ALERT "Goodbye, world\n");
    // cdev_del(&my_cdev); // Remove cdev from the kernel
}

module_init(hello_init);
module_exit(hello_exit);
