#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/ioport.h>
#include <linux/i2c.h>

#define DEVICE_NAME "i2c_custom"

static struct i2c_client *i2c_client;
static struct class *i2c_custom_class;
static struct device *i2c_custom_device;

static int platform_driver_remove(struct platform_device *pdev);
static int my_platform_driver_probe(struct platform_device *pdev);

static ssize_t my_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t my_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static int my_i2c_write_byte(struct i2c_client *client, u8 reg, u8 value);
static int my_i2c_read_byte(struct i2c_client *client, u8 reg, u8 *value);

static const struct of_device_id my_driver_ids[] = {
    { .compatible = DEVICE_NAME },
    { }
};

static struct platform_driver my_driver = {
    .probe = my_platform_driver_probe,
    .remove = platform_driver_remove,
    .driver = {
        .name = DEVICE_NAME,
        .of_match_table = of_match_ptr(my_driver_ids),
    },
};

struct i2c_members {
    uint8_t reg;
};

static struct i2c_members *i2c_data;

static struct device_attribute dev_attr_register_value = {
    .attr = {
        .name = "register-value",
        .mode = 0666,
    },
    .show = my_show,
    .store = my_store
};

static ssize_t my_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    long value;
    int ret;
    u8 reg = 0x10; // specify the register address you want to write to

    printk(KERN_ALERT "store action");

    ret = kstrtol(buf, 0, &value);
    if (ret) {
        printk(KERN_ERR "Invalid input\n");
        return ret;
    }

    printk(KERN_ALERT "store action: %ld", value);
    printk(KERN_ALERT "store action: %ld", value);

    ret = my_i2c_write_byte(i2c_client, reg, (u8)value);
    if (ret) {
        printk(KERN_ERR "Failed to write to the I2C device\n");
        return ret;
    }

    printk(KERN_INFO "Wrote value 0x%02x to register 0x%02x\n", (u8)value, reg);
    printk(KERN_INFO "Wrote value 0x%02x to register 0x%02x\n", (u8)value, reg);
    return count;
}

static ssize_t my_show(struct device *dev, struct device_attribute *attr, char *buf) {
    u8 value;
    int ret;
    u8 reg = 0x10; // specify the register address you want to read from

    printk(KERN_ALERT "show action");

    ret = my_i2c_read_byte(i2c_client, reg, &value);
    if (ret) {
        printk(KERN_ERR "Failed to read from the I2C device\n");
        return ret;
    }

    printk(KERN_INFO "Read value 0x%02x from register 0x%02x\n", value, reg);
    return snprintf(buf, PAGE_SIZE, "0x%02x\n", value);
}

/* Write a single byte to a specific register */
static int my_i2c_write_byte(struct i2c_client *client, u8 reg, u8 value)
{
    u8 buf[2]; int ret;

    buf[0] = reg;
    buf[1] = value;

    ret = i2c_master_send(client, buf, 2);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

/* Read a single byte from a specific register */
static int my_i2c_read_byte(struct i2c_client *client, u8 reg, u8 *value)
{
    int ret;

    ret = i2c_master_send(client, &reg, 1);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to send register address 0x%02x\n", reg);
        return ret;
    }

    ret = i2c_master_recv(client, value, 1);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read byte from register 0x%02x\n", reg);
        return ret;
    }

    return 0;
}

static int my_platform_driver_probe(struct platform_device *pdev) {
    struct device_node *np = pdev->dev.of_node;
    struct i2c_adapter *adapter;
    struct i2c_board_info info;
    int reg_value, i2c_bus;
    int ret;

    if (!np) {
        dev_err(&pdev->dev, "No device tree node\n");
        return -ENODEV;
    }

    // Read 'reg' property (assuming it's a single cell)
    ret = of_property_read_u32(np, "reg", &reg_value);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read 'reg' property\n");
        return ret;
    }

    // Read 'i2c' property (assuming it's a single cell)
    ret = of_property_read_u32(np, "i2c-bus", &i2c_bus);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read 'i2c' property\n");
        return ret;
    }

    printk(KERN_INFO "reg: %d, i2c-bus: %d", reg_value, i2c_bus);
    printk(KERN_INFO "reg: %d, i2c-bus: %d", reg_value, i2c_bus);


    // Find the I2C adapter based on the 'i2c' property
    adapter = i2c_get_adapter(i2c_bus);
    if (!adapter) {
        dev_err(&pdev->dev, "Failed to get I2C adapter for bus %d\n", i2c_bus);
        return -ENODEV;
    }

    memset(&info, 0, sizeof(struct i2c_board_info));
    strlcpy(info.type, "custom_sensor", I2C_NAME_SIZE);
    info.addr = reg_value;

    i2c_client = i2c_new_device(adapter, &info);
    if (!i2c_client) {
        dev_err(&pdev->dev, "Failed to register I2C device\n");
        i2c_put_adapter(adapter);
        return -ENODEV;
    }

    i2c_data = kzalloc(sizeof(struct i2c_members), GFP_KERNEL);
    if (!i2c_data) {
        i2c_unregister_device(i2c_client);
        i2c_put_adapter(adapter);
        return -ENOMEM;
    }

    dev_set_drvdata(&pdev->dev, i2c_data);

    i2c_custom_device = device_create(i2c_custom_class, NULL, MKDEV(0, 0), NULL, "I2C_device%d", 0);
    if (IS_ERR(i2c_custom_device)) {
        ret = PTR_ERR(i2c_custom_device);
        kfree(i2c_data);
        i2c_unregister_device(i2c_client);
        i2c_put_adapter(adapter);
        return ret;
    }

    if (device_create_file(i2c_custom_device, &dev_attr_register_value)) {
        device_destroy(i2c_custom_class, MKDEV(0, 0));
        kfree(i2c_data);
        i2c_unregister_device(i2c_client);
        i2c_put_adapter(adapter);
        return -ENOMEM;
    }

    i2c_put_adapter(adapter);
    return 0;
}

static int platform_driver_remove(struct platform_device *pdev) {
    printk(KERN_INFO "my_platform_device: Removing...\n");
    i2c_unregister_device(i2c_client);

    return 0;
}

static void i2c_custom_exit(void) {
    device_remove_file(i2c_custom_device, &dev_attr_register_value); // Remove the sysfs file

    device_destroy(i2c_custom_class, MKDEV(0, 0)); // Destroy the device
    class_destroy(i2c_custom_class); // Destroy the class

    platform_driver_unregister(&my_driver);
}

static int i2c_custom_init(void) {
    int result;
    i2c_custom_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(i2c_custom_class)) {
        return PTR_ERR(i2c_custom_class);
    }

    result = platform_driver_register(&my_driver);
    if (result) {
        class_destroy(i2c_custom_class);
    }
    return result;
}

module_init(i2c_custom_init);
module_exit(i2c_custom_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seger");
MODULE_DESCRIPTION("My I2C PlatformDriver with Communication Functions");
MODULE_VERSION("1.0");
