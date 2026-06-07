#include "local_storage.h"

#include <string.h>

static char placeholder_device_secret[128];

bool local_storage_get_device_secret(char* buffer, int buffer_size) {
    if (buffer == 0 || buffer_size <= 0 || placeholder_device_secret[0] == '\0') {
        return false;
    }

    strncpy(buffer, placeholder_device_secret, (size_t)buffer_size - 1u);
    buffer[buffer_size - 1] = '\0';
    return true;
}

bool local_storage_set_device_secret(const char* device_secret) {
    if (device_secret == 0 || device_secret[0] == '\0') {
        return false;
    }

    /* TODO: Replace this process-local placeholder with Android private storage. */
    strncpy(placeholder_device_secret, device_secret, sizeof(placeholder_device_secret) - 1u);
    placeholder_device_secret[sizeof(placeholder_device_secret) - 1u] = '\0';
    return true;
}
