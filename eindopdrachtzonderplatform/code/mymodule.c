#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

#define DEVICE_NAME "i2c_custom"
#define I2C_BUS_NUMBER 2 // Change to i2c2

static int my_i2c_write_byte(struct i2c_client *client, u8 reg, u8 value);
static int my_i2c_read_byte(struct i2c_client *client, u8 reg, u8 *value);

static struct class *i2c_custom_class;
static struct device *i2c_custom_device;

struct my_i2c_data {
    struct i2c_client *client;
};

struct i2c_members {
    uint8_t reg;
};

static struct i2c_members *i2c_data;

static ssize_t my_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct i2c_client *client = dev_get_drvdata(dev);
    long value;
    int ret;
    u8 reg = 0x10; // specify the register address you want to write to

    ret = kstrtol(buf, 0, &value);
    if (ret) {
        printk(KERN_ERR "Invalid input\n");
        return ret;
    }

    ret = my_i2c_write_byte(client, reg, (u8)value);
    if (ret) {
        printk(KERN_ERR "Failed to write to the I2C device\n");
        return ret;
    }

    printk(KERN_INFO "Wrote value 0x%02x to register 0x%02x\n", (u8)value, reg);
    return count;
}

static ssize_t my_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct i2c_client *client = dev_get_drvdata(dev);
    u8 value;
    int ret;
    u8 reg = 0x10; // specify the register address you want to read from

    ret = my_i2c_read_byte(client, reg, &value);
    if (ret) {
        printk(KERN_ERR "Failed to read from the I2C device\n");
        return ret;
    }

    printk(KERN_INFO "Read value 0x%02x from register 0x%02x\n", value, reg);
    return snprintf(buf, PAGE_SIZE, "0x%02x\n", value);
}

static struct device_attribute dev_attr_register_value = {
    .attr = {
        .name = "register-value",
        .mode = 0666,
    },
    .show = my_show,
    .store = my_store
};

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

/* Write multiple bytes to the I2C device */
static int my_i2c_write_bytes(struct i2c_client *client, u8 reg, u8 *data, int length)
{
    int ret;
    u8 *buf;

    buf = kmalloc(length + 1, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    buf[0] = reg;
    memcpy(&buf[1], data, length);

    ret = i2c_master_send(client, buf, length + 1);
    kfree(buf);

    if (ret < 0) {
        dev_err(&client->dev, "Failed to write multiple bytes to register 0x%02x\n", reg);
        return ret;
    }

    return 0;
}

/* Read multiple bytes from the I2C device */
static int my_i2c_read_bytes(struct i2c_client *client, u8 reg, u8 *data, int length)
{
    int ret;

    ret = i2c_master_send(client, &reg, 1);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to send register address 0x%02x\n", reg);
        return ret;
    }

    ret = i2c_master_recv(client, data, length);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read multiple bytes from register 0x%02x\n", reg);
        return ret;
    }

    return 0;
}

/* Probe function for I2C driver */
static int my_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct my_i2c_data *data;
    u8 value;
    int ret;

    printk(KERN_INFO "my_i2c_driver: Probing I2C device\n");

    data = devm_kzalloc(&client->dev, sizeof(struct my_i2c_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->client = client;
    i2c_set_clientdata(client, data);

    i2c_data = kzalloc(sizeof(struct i2c_members), GFP_KERNEL);
    if (!i2c_data) {
        dev_err(&client->dev, "Failed to allocate memory\n");
        return -ENOMEM;
    }

    dev_set_drvdata(&client->dev, i2c_data);

    i2c_custom_class = class_create(THIS_MODULE, "i2c_custom");
    if (IS_ERR(i2c_custom_class)) {
        return PTR_ERR(i2c_custom_class);
    }

    i2c_custom_device = device_create(i2c_custom_class, NULL, MKDEV(0, 0), NULL, "I2C device%d", 0);
    if (IS_ERR(i2c_custom_device)) {
        ret = PTR_ERR(i2c_custom_device);
        kfree(i2c_data);
        return ret;
    }

    ret = device_create_file(i2c_custom_device, &dev_attr_register_value);
    if (ret) {
        device_destroy(i2c_custom_class, MKDEV(0, 0));
        kfree(i2c_data);
        return ret;
    }
    return 0;
}

/* Remove function for I2C driver */
static int my_i2c_remove(struct i2c_client *client)
{
    device_remove_file(i2c_custom_device, &dev_attr_register_value);
    device_destroy(i2c_custom_class, MKDEV(0, 0));
    kfree(i2c_data);
    return 0;
}

/* I2C Device ID table */
static const struct i2c_device_id my_i2c_id[] = {
    { DEVICE_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, my_i2c_id);

/* I2C Driver structure */
static struct i2c_driver my_i2c_driver = {
    .driver = {
        .name   = DEVICE_NAME,
	.owner = THIS_MODULE,
    },
    .probe          = my_i2c_probe,
    .remove         = my_i2c_remove,
    .id_table       = my_i2c_id,
};

/* Module initialization and exit functions */
static int __init my_module_init(void)
{
    int ret;

    printk(KERN_INFO "my_module_init");

    ret = i2c_add_driver(&my_i2c_driver);
    if (ret)
        return ret;

    return ret;
}

static void __exit my_module_exit(void)
{
    i2c_del_driver(&my_i2c_driver);
    class_destroy(i2c_custom_class);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seger");
MODULE_DESCRIPTION("My I2C Driver with Communication Functions");
MODULE_VERSION("1.0");
