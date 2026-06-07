#ifndef CAT_CHESS_LOCAL_STORAGE_H
#define CAT_CHESS_LOCAL_STORAGE_H

#include <stdbool.h>

bool local_storage_get_device_secret(char* buffer, int buffer_size);
bool local_storage_set_device_secret(const char* device_secret);

#endif
