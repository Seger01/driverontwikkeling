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

MODULE_LICENSE("Dual BSD/GPL");

/* PIN */
int PIN = 0;

/* base address */
// int GPIO1_ADDR = 0x4804c000;
int GPIO1_ADDR = 0;

/* register offsets in uint32_t sizes */
#define GPIO_OE 0x4D // 0x134 all shifted >> 2
#define GPIO_DATAIN 0x4E // 0x138
#define GPIO_CLEARDATAOUT 0x64 // 0x190
#define GPIO_SETDATAOUT 0x65 // 0x194

/* max size in bytes */
#define GPIO_MAX 0x198

uint32_t* gpio1;
uint32_t oe;

static int gpio_ex_probe(struct platform_device* dev);
static int gpio_ex_remove(struct platform_device* dev);

static const struct of_device_id g_ids[] = {
	{ .compatible = "gpio-extern", },
	{ } // ends with empty; MUST be last member
};

struct platform_driver g_driver = {
	.probe = gpio_ex_probe, // obliged
	.remove = gpio_ex_remove, // obliged
	// .shutdown // optional
	// .suspend // optional
	// .suspend_late // optional
	// .resume_early // optional
	// .resume // optional
	.driver = {
		.name = "gpio-extern", // name of the driver
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_ids), // compatible device id
	}
};

static ssize_t driver_attribute_show (struct device_driver* drv, char* buf)
{
	return sprintf(buf, "value: %d\n", 1324); //
}
static ssize_t driver_attribute_store(struct device_driver* drv, const char* buf, size_t count)
{
	int retval = count;
	unsigned int num = 1234;

	if (sscanf(buf, "%u", &num) < 1) {
		// error handling
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

void toggleLed(void){
        static bool ledstate = false;

        setLed(ledstate);

        ledstate = !ledstate;
}

bool getLedState(void){
        gpio1 = ioremap(GPIO1_ADDR , GPIO_MAX * sizeof(uint32_t));
        if (!gpio1) {
                pr_err("Failed to map GPIO1 address\n");
                return -EFAULT;
        }

        int gpio_value = ioread32(gpio1 + GPIO_DATAIN);
        gpio_value = (gpio_value >> PIN);
        gpio_value = (gpio_value & 0x01);
        // pr_alert("GPIO1 value: 0x%x\n", gpio_value);

        // printk(KERN_ALERT "ledState: %d\n", gpio_value);
        iounmap(gpio1);

        return gpio_value;
}


void get_led_info(void){


}

static int gpio_ex_probe(struct platform_device* dev){
	printk("gpio_ex_probe\n");
	printk("gpio_ex_probe: %s\n", dev->name);

	u32 led_values[3] = {0};
	u32 gpio_reg_values[2] = {0};

	{
		// device tree node
		struct device_node* node = dev->dev.of_node;

		int i = 0;
		u32 val;
		int elems;
		elems = of_property_count_u32_elems(node, "gpios");

		for (i = 0; i < elems; i++){
			of_property_read_u32_index(node, "gpios", i, &val);
			led_values[i] = val;
			printk("val: %d\n", val);
		}
	}

	phandle handle = led_values[0];
	PIN = led_values[1];

	{
		struct device_node* gpio_node = of_find_node_by_phandle(handle);

		int i = 0;
		u32 val;
		int elems;
		elems = of_property_count_u32_elems(gpio_node, "reg");

		for (i = 0; i < elems; i++){
			of_property_read_u32_index(gpio_node, "reg", i, &val);
			gpio_reg_values[i] = val;
			printk("val: %d\n", val);
		}
	}
	GPIO1_ADDR = gpio_reg_values[0];




	return 0;
}
static int gpio_ex_remove(struct platform_device* dev){
	printk("gpio_ex_remove\n");
	return 0;
}

static int gpio_ex_init(void)
{
	printk("gpio_ex_init\n");
	int result;
	result = platform_driver_register(&g_driver);

	// Ergens nadat de driver succesvol geregistreerd is
	result = driver_create_file(&(g_driver.driver), &dvr_attr_demo); // device_driver is part of platform_driver

	return result;
}

static void gpio_ex_exit(void)
{
	printk("gpio_ex_exit\n");

	// Vlak voordat de driver ge-unregistreerd is...
	driver_remove_file(&(g_driver.driver), &dvr_attr_demo); // device_driver is part of platform_driver

	platform_driver_unregister(&g_driver);

}


module_init(gpio_ex_init);
module_exit(gpio_ex_exit);
