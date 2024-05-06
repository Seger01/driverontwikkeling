#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>

MODULE_LICENSE("Dual BSD/GPL");

// static int my_param = 0;
// module_param_named(my_param, my_param, int, 0644); // Define the module parameter
static struct cdev my_cdev;

/* PIN */
#define PIN 19

/* base address */
#define GPIO1_ADDR 0x4804c000

/* register offsets in uint32_t sizes */
#define GPIO_OE 0x4D // 0x134
#define GPIO_DATAIN 0x4E // 0x138
#define GPIO_CLEARDATAOUT 0x64 // 0x190
#define GPIO_SETDATAOUT 0x65 // 0x194

/* max size in bytes */
#define GPIO_MAX 0x198

uint32_t* gpio1;
uint32_t oe;


void setPinMode(void){
	/* output instellen */
	gpio1 = ioremap( GPIO1_ADDR, GPIO_MAX * sizeof(uint32_t) );
	barrier();
	oe = ioread32( gpio1 + GPIO_OE );
	rmb();
	iowrite32( (oe & (~(1<<PIN))), gpio1 + GPIO_OE );
	wmb(); // write memory barrier
	iounmap(gpio1);
}

void setLed(bool state){
	if (state){
		/* ledje aan en uit zetten */
		gpio1 = ioremap(GPIO1_ADDR, GPIO_MAX);
		barrier();
		iowrite32( (1<<PIN), gpio1 + GPIO_SETDATAOUT ); // Pin 19 aan

		// iowrite32( (1<<PIN), gpio1 + GPIO_CLEARDATAOUT ); // Pin 19 uit
		wmb(); // write memory barrier
		iounmap(gpio1);
	} else {
		/* ledje aan en uit zetten */
		gpio1 = ioremap(GPIO1_ADDR, GPIO_MAX);
		barrier();
		// iowrite32( (1<<PIN), gpio1 + GPIO_SETDATAOUT ); // Pin 19 aan

		iowrite32( (1<<PIN), gpio1 + GPIO_CLEARDATAOUT ); // Pin 19 uit
		wmb(); // write memory barrier
		iounmap(gpio1);
	}
}

static int hello_open(struct inode* inode, struct file* file) {
    printk(KERN_ALERT "hello_open()\n");
    return 0;
}

static int hello_release(struct inode* inode, struct file* file) {
    printk(KERN_ALERT "hello_release()\n");
    return 0;
}

static ssize_t hello_read(struct file* file, char __user* buf, size_t lbuf, loff_t* ppos) {
//     static bool isRead = false;
// 
//     char temp_write_buffer[100] = {0};
//     printk(KERN_ALERT "hello_read() %zu\n", lbuf);
// 
//     if (isRead == true) {
//         isRead = false;
//         return 0;
//     }
// 
//     if (lbuf < 3) {
//         printk(KERN_ALERT "Insufficient buffer size\n");
//         return -EINVAL;
//     }
// 
//     temp_write_buffer[0] = 'h';
//     temp_write_buffer[1] = '2';
//     temp_write_buffer[2] = '\n';
// 
//     if (copy_to_user(buf, temp_write_buffer, 3)) {
//         printk(KERN_ALERT "Failed to copy data to user space\n");
//         return -EFAULT;
//     }
// 
//     *ppos += 3;
// 
//     isRead = true;
//     return 3;
    setLed(true);
    return 0;
}

static ssize_t hello_write(struct file* file, const char __user* buf, size_t lbuf, loff_t* ppos) {
    printk(KERN_ALERT "hello_write()\n");
    setLed(false);
    return lbuf;
}

struct file_operations fops = {
    .read = hello_read,
    .write = hello_write,
    .open = hello_open,
    .release = hello_release,
};

static int hello_init(void) {
    int result;
    dev_t dev = MKDEV(500, 0);
    struct cdev* my_cdev = cdev_alloc();


    printk(KERN_ALERT "Hello, world\n");
    // printk(KERN_INFO "Hello, world! My param value: %d\n", my_param);

    // Allocate a range of character device numbers
//     ret = alloc_chrdev_region(&dev, 0, 1, "my_device");
//     if (ret < 0) {
//         printk(KERN_ALERT "Failed to allocate character device region\n");
//         return ret;
//     }
//     printk(KERN_INFO "Allocated major number: %d\n", MAJOR((unsigned int)dev));

    // cdev_init(&my_cdev, &fops);
    my_cdev->ops = &fops;

    result = cdev_add(my_cdev, dev, 1);
    printk(KERN_ALERT "result: %d", result);
    if (result < 0) {
        printk(KERN_ALERT "Failed to register cdev\n");
        return result;
    }

    printk(KERN_ALERT "Registered character device\n");
    setPinMode();
    return 0;
}

static void hello_exit(void) {
    printk(KERN_ALERT "Goodbye, world\n");
    cdev_del(&my_cdev);
}

module_init(hello_init);
module_exit(hello_exit);
