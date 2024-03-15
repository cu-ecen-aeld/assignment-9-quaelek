#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aesd-circular-buffer.h"

void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer) {
    memset(buffer, 0, sizeof(struct aesd_circular_buffer));
}

void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry) {
    // Check if the buffer is full
    if(buffer->full) {
        // Overwrite the oldest entry if the buffer is full
        buffer->entries[buffer->out_offs] = *add_entry;
        buffer->out_offs = (buffer->out_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    } else {
        // Add new entry
        buffer->entries[buffer->in_offs] = *add_entry;
        buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        if(buffer->in_offs == buffer->out_offs) {
            buffer->full = true;
        }
    }
}

struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn) {
    size_t current_offset = 0;
    size_t index = buffer->out_offs;
    struct aesd_buffer_entry *entry = NULL;

    // Check if buffer is not empty by comparing in and out offsets
    if(!buffer->full && (buffer->in_offs == buffer->out_offs)) {
        // Buffer is empty
        return NULL;
    }

    do {
        entry = &buffer->entries[index];

        if((char_offset >= current_offset) && (char_offset < current_offset + entry->size)) {
            // Found the entry containing the char_offset
            *entry_offset_byte_rtn = char_offset - current_offset;
            return entry;
        }

        current_offset += entry->size;
        index = (index + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

    } while(index != buffer->in_offs);

    // char_offset is beyond the current data in the buffer
    return NULL;
}
