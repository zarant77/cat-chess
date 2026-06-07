#ifndef CAT_CHESS_PLATFORM_ANDROID_H
#define CAT_CHESS_PLATFORM_ANDROID_H

#include <android/native_activity.h>
#include <stddef.h>

void platform_android_on_create(
        ANativeActivity* activity,
        void* saved_state,
        size_t saved_state_size
);

#endif
