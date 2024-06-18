#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>
/* External declaration */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h> // struct of_device_id
#include <linux/of.h> // of_match_ptr macro
#include <linux/ioport.h> // struct resource
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");

uint32_t* gpio1;
uint32_t oe;

static ssize_t showBrightness(struct device *dev, struct device_attribute *attr, char *buf);

static ssize_t storeBrightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static ssize_t showStatus(struct device *dev, struct device_attribute *attr, char *buf);

static ssize_t storeStatus(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static struct class *gpio_ex_class;
static struct device *gpio_ex_device;

struct stored_attributes{
	uint8_t brightness;
	uint8_t status;
};


static struct stored_attributes *led_data;

struct device_attribute dev_attr_brightness = {
	.attr = {
		.name = "brightness",
		.mode = 00666,
	},
	.show = showBrightness,
	.store = storeBrightness
};

struct device_attribute dev_attr_status = {
	.attr = {
		.name = "status",
		.mode = 00666,
	},
	.show = showStatus,
	.store = storeStatus
};


static ssize_t showBrightness(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%u\n", led_data->brightness);
}


static ssize_t storeBrightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	int brightness;
	if (sscanf(buf, "%d", &brightness) == 1) {
		led_data->brightness = brightness;
	}
	return count;
}

static ssize_t showStatus(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%u\n", led_data->status);
}

static ssize_t storeStatus(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	int status;
	if (sscanf(buf, "%d", &status) == 1) {
		led_data->status = status;
	}
	return count;
}


static ssize_t driver_attribute_show (struct device_driver* drv, char* buf)
{
	return sprintf(buf, "value: %d\n", 1324);
}

static ssize_t driver_attribute_store(struct device_driver* drv, const char* buf, size_t count)
{
	int retval = count;
	unsigned int num = 1234;

	if (sscanf(buf, "%u", &num) < 1) {

	}
	printk("driver_attribute_store: %d\n", num);
	return retval;
}

struct driver_attribute dvr_attr_demo = {
	.attr = {
		.name = "demo",
		.mode = 00666,
	},
	.show = driver_attribute_show,
	.store = driver_attribute_store
};

static int gpio_ex_probe(struct platform_device* dev)
{
	led_data = kzalloc(sizeof(struct stored_attributes), GFP_KERNEL);
	dev_set_drvdata(&dev->dev, led_data);
	gpio_ex_device = device_create(gpio_ex_class, NULL, MKDEV(0, 0), NULL, "gpio_ex%d", 0);
	device_create_file(gpio_ex_device, &dev_attr_brightness);
	device_create_file(gpio_ex_device, &dev_attr_status);
	return 0;
}

static int gpio_ex_remove(struct platform_device* dev){
	printk("gpio_ex_remove\n");
	device_remove_file(gpio_ex_device, &dev_attr_brightness);
	device_remove_file(gpio_ex_device, &dev_attr_status);
	device_destroy(gpio_ex_class, MKDEV(0, 0));
	class_destroy(gpio_ex_class);

	kfree(led_data);
	return 0;
}

static const struct of_device_id g_ids[] = {
	{ .compatible = "gpio-extern", },
	{ } 
};

struct platform_driver g_driver = {
	.probe = gpio_ex_probe, 
	.remove = gpio_ex_remove,

	.driver = {
		.name = "gpio-extern",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_ids),
	}
};

static void gpio_ex_exit(void)
{
	printk("gpio_ex_exit\n");

		        
	// driver_remove_file(&(g_driver.driver), &dvr_attr_demo); 

	platform_driver_unregister(&g_driver);

}

static int gpio_ex_init(void)
{
	printk("gpio_ex_init\n");
	int result;
	gpio_ex_class = class_create(THIS_MODULE, "led_extern");

	result = platform_driver_register(&g_driver);

	printk(KERN_ALERT "%d\n", result);
	return result;
}


module_init(gpio_ex_init);
module_exit(gpio_ex_exit);
