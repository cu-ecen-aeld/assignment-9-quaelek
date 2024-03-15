#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "aesd_ioctl.h"

#define DEVICE_NAME "aesdchar"
#define CLASS_NAME "aesd"

static int majorNumber;
static struct class* aesdClass = NULL;
static struct device* aesdDevice = NULL;
static struct cdev aesdCdev;

struct aesd_buffer {
    char *data;
    size_t size;
};

struct aesd_dev {
    struct aesd_buffer *buffer;
    unsigned int buffer_count;
    unsigned int current_position;
};

static struct aesd_dev aesd_device;

static int aesd_open(struct inode *inode, struct file *file) {
    file->private_data = &aesd_device;
    return 0;
}

static int aesd_release(struct inode *inode, struct file *file) {
    return 0;
}

static long aesd_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct aesd_dev *dev = file->private_data;
    struct aesd_seekto seekto;
    unsigned int new_position, i, offset = 0;

    if (_IOC_TYPE(cmd) != AESD_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > AESDCHAR_IOC_MAXNR) return -ENOTTY;

    switch (cmd) {
        case AESDCHAR_IOCSEEKTO:
            if (copy_from_user(&seekto, (struct aesd_seekto __user *)arg, sizeof(seekto))) {
                return -EFAULT;
            }
            if (seekto.write_cmd >= dev->buffer_count) {
                return -EINVAL;
            }
            for (i = 0; i <= seekto.write_cmd; i++) {
                if (i == seekto.write_cmd) {
                    if (seekto.write_cmd_offset > dev->buffer[i].size) {
                        return -EINVAL;
                    }
                    offset += seekto.write_cmd_offset;
                    break;
                }
                offset += dev->buffer[i].size;
            }
            dev->current_position = offset;
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = aesd_open,
    .release = aesd_release,
    .unlocked_ioctl = aesd_ioctl,
};

static int __init aesd_init(void) {
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "AESD failed to register a major number\n");
        return majorNumber;
    }
    aesdClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(aesdClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(aesdClass);
    }
    aesdDevice = device_create(aesdClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(aesdDevice)) {
        class_destroy(aesdClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(aesdDevice);
    }
    cdev_init(&aesdCdev, &fops);
    cdev_add(&aesdCdev, MKDEV(majorNumber, 0), 1);
    memset(&aesd_device, 0, sizeof(struct aesd_dev));
    printk(KERN_INFO "AESD: device class created correctly\n");
    return 0;
}

static void __exit aesd_exit(void) {
    cdev_del(&aesdCdev);
    device_destroy(aesdClass, MKDEV(majorNumber, 0));
    class_unregister(aesdClass);
    class_destroy(aesdClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "AESD: goodbye from the LKM!\n");
}

module_init(aesd_init);
module_exit(aesd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("A simple Linux char driver for the AESD Assignment");
MODULE_VERSION("0.1");

