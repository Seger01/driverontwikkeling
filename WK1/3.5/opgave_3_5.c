#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct cdev my_cdev;

static int hello_open(struct inode* inode, struct file* file) {
    printk(KERN_ALERT "hello_open()\n");
    return 0;
}

static int hello_release(struct inode* inode, struct file* file) {
    printk(KERN_ALERT "hello_release()\n");
    return 0;
}

static ssize_t hello_read(struct file* file, char __user* buf, size_t lbuf, loff_t* ppos) {
    static bool isRead = false;
    printk(KERN_ALERT "hello_read() %zu\n", lbuf);

    if (isRead == true) {
        return -ENODATA; // Return error indicating insufficient buffer size
    }

    // Check if buffer is large enough to hold data
    if (lbuf < 2) {
        printk(KERN_ALERT "Insufficient buffer size\n");
        return -EINVAL; // Return error indicating insufficient buffer size
    }

    // Copy data to user space buffer
    if (copy_to_user(buf, "h2", 2)) {
        printk(KERN_ALERT "Failed to copy data to user space\n");
        return -EFAULT; // Return error indicating copy failure
    }

    // Update file position
    *ppos += 2;

    // return -ENODATA; // Return error indicating insufficient buffer size
    isRead = true;
    return 2; // Return the number of bytes read
}

// static ssize_t hello_read(struct file* file, char __user* buf, size_t lbuf, loff_t* ppos) {
//     printk(KERN_ALERT "hello_read() %zu\n", lbuf);
//
//     // Check if buffer is large enough to hold data
//     if (lbuf < 2) {
//         printk(KERN_ALERT "Insufficient buffer size\n");
//         return -EINVAL; // Return error indicating insufficient buffer size
//     }
//
//     // Copy data to user space buffer
//     if (copy_to_user(buf, "h2", 2)) {
//         printk(KERN_ALERT "Failed to copy data to user space\n");
//         return -EFAULT; // Return error indicating copy failure
//     }
//
//     return 2; // Return the number of bytes read
// }

// static ssize_t hello_read(struct file* file, char __user* buf, size_t lbuf, loff_t* ppos) {
//     printk(KERN_ALERT "hello_read() %d\n", (int)lbuf);
//     buf[0] = 'h';
//     buf[1] = '2';
//     lbuf = 2;
//     return 2;
// }

static ssize_t hello_write(struct file* file, const char __user* buf, size_t lbuf, loff_t* ppos) {
    printk(KERN_ALERT "hello_write()\n");
    return lbuf;
}

struct file_operations fops = {
    .read = hello_read,
    .write = hello_write,
    .open = hello_open,
    .release = hello_release,
};

static int hello_init(void) {
    // dev_t device_number;
    // int result;
    // device_number = MKDEV(major, minor);
    // device = cdev_alloc() result = register_chrdev_region(device_number, amount, driver_name);
    // return result;

    int result;
    dev_t dev = MKDEV(500, 0); // Device number
    struct cdev* my_cdev = cdev_alloc();

    printk(KERN_ALERT "Hello, world\n");

    // Allocate memory for cdev
    // cdev_init(&my_cdev, &fops);
    my_cdev->ops = &fops;

    // Add cdev to the kernel
    result = cdev_add(my_cdev, dev, 1);
    printk(KERN_ALERT "result: %d", result);
    if (result < 0) {
        printk(KERN_ALERT "Failed to register cdev\n");
        return result;
    }

    printk(KERN_ALERT "Registered character device\n");
    return 0;
}

static void hello_exit(void) {
    printk(KERN_ALERT "Goodbye, world\n");
    cdev_del(&my_cdev); // Remove cdev from the kernel
}

module_init(hello_init);
module_exit(hello_exit);
