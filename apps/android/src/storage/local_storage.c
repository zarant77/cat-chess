#include "local_storage.h"

#include <stdio.h>
#include <string.h>

#define LOCAL_STORAGE_PATH_MAX 512
#define LOCAL_STORAGE_DEVICE_SECRET_FILE "device_secret.txt"

static char local_storage_base_path[LOCAL_STORAGE_PATH_MAX];

void local_storage_set_base_path(const char* base_path) {
    if (base_path == 0 || base_path[0] == '\0') {
        local_storage_base_path[0] = '\0';
        return;
    }

    strncpy(local_storage_base_path, base_path, sizeof(local_storage_base_path) - 1u);
    local_storage_base_path[sizeof(local_storage_base_path) - 1u] = '\0';
}

static int local_storage_device_secret_path(char* buffer, int buffer_size) {
    int written;

    if (buffer == 0 || buffer_size <= 0 || local_storage_base_path[0] == '\0') {
        return 0;
    }

    written = snprintf(
            buffer,
            (size_t)buffer_size,
            "%s/%s",
            local_storage_base_path,
            LOCAL_STORAGE_DEVICE_SECRET_FILE
    );

    return written > 0 && written < buffer_size;
}

bool local_storage_get_device_secret(char* buffer, int buffer_size) {
    char path[LOCAL_STORAGE_PATH_MAX];
    FILE* file;
    size_t count;

    if (buffer == 0 || buffer_size <= 0 || !local_storage_device_secret_path(path, sizeof(path))) {
        return false;
    }

    file = fopen(path, "rb");
    if (file == 0) {
        return false;
    }

    count = fread(buffer, 1u, (size_t)buffer_size - 1u, file);
    fclose(file);

    buffer[count] = '\0';
    while (count > 0 && (buffer[count - 1] == '\n' || buffer[count - 1] == '\r' || buffer[count - 1] == ' ')) {
        count -= 1;
        buffer[count] = '\0';
    }

    return buffer[0] != '\0';
}

bool local_storage_set_device_secret(const char* device_secret) {
    char path[LOCAL_STORAGE_PATH_MAX];
    FILE* file;
    size_t length;

    if (device_secret == 0
            || device_secret[0] == '\0'
            || !local_storage_device_secret_path(path, sizeof(path))) {
        return false;
    }

    file = fopen(path, "wb");
    if (file == 0) {
        return false;
    }

    length = strlen(device_secret);
    if (length > 0) {
        fwrite(device_secret, 1u, length, file);
    }
    fclose(file);
    return length > 0;
}
