#include <linux/fs.h>

loff_t aesd_llseek(struct file *file, loff_t offset, int whence) {
    struct aesd_dev *dev = file->private_data;
    loff_t newpos;
    switch (whence) {
        case SEEK_SET:
            newpos = offset;
            break;
        case SEEK_CUR:
            newpos = file->f_pos + offset;
            break;
        case SEEK_END:
            // For SEEK_END, you would need to calculate the total size. This is a placeholder.
            newpos = aesd_circular_buffer_total_size(dev->buffer) + offset;
            break;
        default:
            return -EINVAL;
    }
    if (newpos < 0) return -EINVAL;
    size_t entry_offset_byte;
    struct aesd_buffer_entry *entry = aesd_circular_buffer_find_entry_offset_for_fpos(dev->buffer, newpos, &entry_offset_byte);
    if (entry) {
        file->f_pos = newpos;
        return newpos;
    }
    return -EINVAL;
}

const struct file_operations aesd_fops = {
    .owner = THIS_MODULE,
    .llseek = aesd_llseek,
    .read = aesd_read,
    .write = aesd_write,
};

