#include "platform_android.h"

#include <android/input.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "../config.h"
#include "../app/app.h"
#include "../audio/audio.h"
#include "../input/input.h"
#include "../online/cat_chess_api.h"
#include "../renderer/renderer.h"
#include "../sprites/generated_sprite.h"
#include "../storage/local_storage.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, CAT_CHESS_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, CAT_CHESS_LOG_TAG, __VA_ARGS__)

#ifndef ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT
#define ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT 0
#endif

typedef struct AndroidPlatform {
    ANativeActivity* activity;
    ANativeWindow* window;
    pthread_mutex_t window_mutex;
    AInputQueue* input_queue;
    AInputQueue* attached_input_queue;
    pthread_mutex_t input_queue_mutex;
    int input_cancel_requested;
    pthread_t thread;
    volatile int loop_running;
    AppState app;
    InputState input;
    double fps_elapsed;
    double fps_frame_time_total;
    int fps_frame_count;
    int null_window_logged;
    int reset_frame_time;
    int buffer_format_logged;
    int finish_requested;
} AndroidPlatform;

static AndroidPlatform* platform_from_activity(ANativeActivity* activity) {
    return (AndroidPlatform*)activity->instance;
}

static JNIEnv* platform_get_jni_env(ANativeActivity* activity, int* did_attach) {
    JNIEnv* env = NULL;

    if (did_attach != NULL) {
        *did_attach = 0;
    }
    if (activity == NULL || activity->vm == NULL) {
        return NULL;
    }

    if ((*activity->vm)->GetEnv(activity->vm, (void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        return env;
    }

    if ((*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL) != JNI_OK) {
        return NULL;
    }
    if (did_attach != NULL) {
        *did_attach = 1;
    }
    return env;
}

static void platform_clear_jni_exception(JNIEnv* env) {
    if (env != NULL && (*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }
}

static jclass platform_load_app_class(JNIEnv* env, jobject activity, const char* class_name_value) {
    jclass activity_class;
    jmethodID get_class_loader_method;
    jobject class_loader;
    jclass class_loader_class;
    jmethodID load_class_method;
    jstring class_name;
    jclass loaded_class = NULL;

    if (env == NULL || activity == NULL || class_name_value == NULL) {
        return NULL;
    }

    activity_class = (*env)->GetObjectClass(env, activity);
    if (activity_class == NULL) {
        platform_clear_jni_exception(env);
        return NULL;
    }

    get_class_loader_method = (*env)->GetMethodID(env, activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
    if (get_class_loader_method == NULL) {
        platform_clear_jni_exception(env);
        (*env)->DeleteLocalRef(env, activity_class);
        return NULL;
    }

    class_loader = (*env)->CallObjectMethod(env, activity, get_class_loader_method);
    (*env)->DeleteLocalRef(env, activity_class);
    if (class_loader == NULL || (*env)->ExceptionCheck(env)) {
        platform_clear_jni_exception(env);
        return NULL;
    }

    class_loader_class = (*env)->FindClass(env, "java/lang/ClassLoader");
    if (class_loader_class == NULL) {
        platform_clear_jni_exception(env);
        (*env)->DeleteLocalRef(env, class_loader);
        return NULL;
    }

    load_class_method = (*env)->GetMethodID(env, class_loader_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (load_class_method == NULL) {
        platform_clear_jni_exception(env);
        (*env)->DeleteLocalRef(env, class_loader_class);
        (*env)->DeleteLocalRef(env, class_loader);
        return NULL;
    }

    class_name = (*env)->NewStringUTF(env, class_name_value);
    if (class_name != NULL) {
        loaded_class = (jclass)(*env)->CallObjectMethod(env, class_loader, load_class_method, class_name);
        (*env)->DeleteLocalRef(env, class_name);
        if ((*env)->ExceptionCheck(env)) {
            platform_clear_jni_exception(env);
            loaded_class = NULL;
        }
    } else {
        platform_clear_jni_exception(env);
    }

    (*env)->DeleteLocalRef(env, class_loader_class);
    (*env)->DeleteLocalRef(env, class_loader);
    return loaded_class;
}

static jclass platform_load_soft_keyboard_class(JNIEnv* env, ANativeActivity* activity) {
    return platform_load_app_class(env, activity->clazz, "com.catemup.catchess.SoftKeyboard");
}

static void platform_poll_soft_keyboard_text(AndroidPlatform* platform) {
    JNIEnv* env;
    int did_attach = 0;
    jclass keyboard_class;
    jmethodID take_text_method;
    jmethodID take_backspaces_method;
    jstring text_string;
    const char* text_chars;
    jint backspaces;

    if (platform == NULL || platform->activity == NULL || platform->activity->clazz == NULL) {
        return;
    }

    env = platform_get_jni_env(platform->activity, &did_attach);
    if (env == NULL) {
        return;
    }

    keyboard_class = platform_load_soft_keyboard_class(env, platform->activity);
    if (keyboard_class == NULL) {
        goto cleanup;
    }

    take_text_method = (*env)->GetStaticMethodID(env, keyboard_class, "takePendingText", "()Ljava/lang/String;");
    take_backspaces_method = (*env)->GetStaticMethodID(env, keyboard_class, "takePendingBackspaces", "()I");
    if (take_text_method == NULL || take_backspaces_method == NULL) {
        platform_clear_jni_exception(env);
        goto cleanup_keyboard_class;
    }

    backspaces = (*env)->CallStaticIntMethod(env, keyboard_class, take_backspaces_method);
    if ((*env)->ExceptionCheck(env)) {
        platform_clear_jni_exception(env);
        backspaces = 0;
    }
    for (jint index = 0; index < backspaces; ++index) {
        input_handle_text_backspace(&platform->input);
    }

    text_string = (jstring)(*env)->CallStaticObjectMethod(env, keyboard_class, take_text_method);
    if (text_string != NULL && !(*env)->ExceptionCheck(env)) {
        text_chars = (*env)->GetStringUTFChars(env, text_string, NULL);
        if (text_chars != NULL) {
            for (int index = 0; text_chars[index] != '\0'; ++index) {
                input_handle_text_char(&platform->input, text_chars[index]);
            }
            (*env)->ReleaseStringUTFChars(env, text_string, text_chars);
        }
        (*env)->DeleteLocalRef(env, text_string);
    } else {
        platform_clear_jni_exception(env);
    }

cleanup_keyboard_class:
    (*env)->DeleteLocalRef(env, keyboard_class);
cleanup:
    if (did_attach) {
        (*platform->activity->vm)->DetachCurrentThread(platform->activity->vm);
    }
}

static int platform_show_soft_keyboard_java(ANativeActivity* activity) {
    JNIEnv* env;
    int did_attach = 0;
    int shown = 0;
    jclass keyboard_class;
    jmethodID show_method;

    if (activity == NULL || activity->clazz == NULL) {
        return 0;
    }

    env = platform_get_jni_env(activity, &did_attach);
    if (env == NULL) {
        return 0;
    }

    keyboard_class = platform_load_soft_keyboard_class(env, activity);
    if (keyboard_class == NULL) {
        goto cleanup;
    }

    show_method = (*env)->GetStaticMethodID(env, keyboard_class, "show", "(Landroid/app/Activity;)V");
    if (show_method != NULL) {
        (*env)->CallStaticVoidMethod(env, keyboard_class, show_method, activity->clazz);
        shown = !(*env)->ExceptionCheck(env);
        platform_clear_jni_exception(env);
    } else {
        platform_clear_jni_exception(env);
    }

    (*env)->DeleteLocalRef(env, keyboard_class);
cleanup:
    if (did_attach) {
        (*activity->vm)->DetachCurrentThread(activity->vm);
    }
    return shown;
}

static double platform_now_seconds(void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    return (double)time.tv_sec + ((double)time.tv_nsec / 1000000000.0);
}

static void platform_sleep_seconds(double seconds) {
    struct timespec sleep_time;

    if (seconds <= 0.0) {
        return;
    }

    sleep_time.tv_sec = (time_t)seconds;
    sleep_time.tv_nsec = (long)((seconds - (double)sleep_time.tv_sec) * 1000000000.0);

    nanosleep(&sleep_time, NULL);
}

static int platform_motion_action_index(int action) {
    return (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
           >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
}

static void platform_handle_motion_event(
        AndroidPlatform* platform,
        AInputEvent* event,
        float screen_width
) {
    int action = AMotionEvent_getAction(event);
    int action_type = action & AMOTION_EVENT_ACTION_MASK;
    int action_index = platform_motion_action_index(action);
    size_t pointer_count = AMotionEvent_getPointerCount(event);

    if (action_type == AMOTION_EVENT_ACTION_DOWN
            || action_type == AMOTION_EVENT_ACTION_POINTER_DOWN
            || action_type == AMOTION_EVENT_ACTION_UP
            || action_type == AMOTION_EVENT_ACTION_POINTER_UP) {
        int pointer_id;
        float x;
        float y;

        if (action_index < 0 || (size_t)action_index >= pointer_count) {
            return;
        }

        pointer_id = AMotionEvent_getPointerId(event, action_index);
        x = AMotionEvent_getX(event, action_index);
        y = AMotionEvent_getY(event, action_index);

        input_handle_touch(&platform->input, action_type == AMOTION_EVENT_ACTION_UP
                || action_type == AMOTION_EVENT_ACTION_POINTER_UP
                ? INPUT_TOUCH_UP
                : INPUT_TOUCH_DOWN,
                pointer_id,
                x,
                y,
                screen_width);

        return;
    }

    if (action_type == AMOTION_EVENT_ACTION_MOVE) {
        for (size_t pointer_index = 0; pointer_index < pointer_count; ++pointer_index) {
            int pointer_id = AMotionEvent_getPointerId(event, pointer_index);
            float x = AMotionEvent_getX(event, pointer_index);
            float y = AMotionEvent_getY(event, pointer_index);

            input_handle_touch(&platform->input, INPUT_TOUCH_MOVE, pointer_id, x, y, screen_width);
        }
        return;
    }

    if (action_type == AMOTION_EVENT_ACTION_CANCEL) {
        input_handle_touch(&platform->input, INPUT_TOUCH_CANCEL, -1, 0.0f, 0.0f, screen_width);
    }
}

static int platform_handle_key_event(AndroidPlatform* platform, AInputEvent* event) {
    int key_code = AKeyEvent_getKeyCode(event);
    char text_char = '\0';

    if (key_code == AKEYCODE_BACK && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
        input_handle_back(&platform->input);
        return 1;
    }

    if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
        if (key_code == AKEYCODE_DEL) {
            input_handle_text_backspace(&platform->input);
            return 1;
        }

        if (key_code >= AKEYCODE_A && key_code <= AKEYCODE_Z) {
            text_char = (char)('A' + key_code - AKEYCODE_A);
        } else if (key_code >= AKEYCODE_0 && key_code <= AKEYCODE_9) {
            text_char = (char)('0' + key_code - AKEYCODE_0);
        }

        if (text_char != '\0') {
            input_handle_text_char(&platform->input, text_char);
            return 1;
        }
    }

    return key_code == AKEYCODE_BACK;
}

static void platform_process_input(AndroidPlatform* platform, float screen_width) {
    AInputQueue* queue;
    AInputEvent* event = NULL;

    pthread_mutex_lock(&platform->input_queue_mutex);
    queue = platform->input_queue;

    if (platform->input_cancel_requested) {
        input_handle_touch(&platform->input, INPUT_TOUCH_CANCEL, -1, 0.0f, 0.0f, screen_width);
        platform->input_cancel_requested = 0;
    }

    if (platform->attached_input_queue != queue) {
        if (platform->attached_input_queue != NULL) {
            AInputQueue_detachLooper(platform->attached_input_queue);
            platform->attached_input_queue = NULL;
        }

        if (queue != NULL) {
            ALooper* looper = ALooper_forThread();
            if (looper != NULL) {
                AInputQueue_attachLooper(queue, looper, 1, NULL, NULL);
                platform->attached_input_queue = queue;
            }
        }
    }

    if (queue == NULL) {
        pthread_mutex_unlock(&platform->input_queue_mutex);
        return;
    }

    while (AInputQueue_getEvent(queue, &event) >= 0) {
        int handled = 0;

        if (AInputQueue_preDispatchEvent(queue, event)) {
            AInputQueue_finishEvent(queue, event, 0);
            continue;
        }

        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            platform_handle_motion_event(platform, event, screen_width);
            handled = 1;
        } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
            handled = platform_handle_key_event(platform, event);
        }

        AInputQueue_finishEvent(queue, event, handled);
    }

    pthread_mutex_unlock(&platform->input_queue_mutex);
}

static ANativeWindow* platform_acquire_window(AndroidPlatform* platform) {
    ANativeWindow* window;
    int should_log_null_window = 0;

    pthread_mutex_lock(&platform->window_mutex);
    window = platform->window;
    if (window == NULL) {
        if (!platform->null_window_logged) {
            platform->null_window_logged = 1;
            should_log_null_window = 1;
        }
    } else {
        ANativeWindow_acquire(window);
        platform->null_window_logged = 0;
    }
    pthread_mutex_unlock(&platform->window_mutex);

    if (should_log_null_window) {
        LOGI("Render skipped because window is null");
    }

    return window;
}

static int platform_draw(AndroidPlatform* platform, float dt) {
    ANativeWindow* window;
    ANativeWindow_Buffer buffer;
    int should_show_keyboard;

    if (platform == NULL) {
        return 0;
    }

    window = platform_acquire_window(platform);
    if (window == NULL) {
        return 0;
    }

    if (ANativeWindow_lock(window, &buffer, NULL) != 0) {
        ANativeWindow_release(window);
        LOGE("Failed to lock native window");
        return 0;
    }

    if (!platform->buffer_format_logged) {
        LOGI(
                "Window buffer locked format=%d width=%d height=%d stride=%d",
                buffer.format,
                buffer.width,
                buffer.height,
                buffer.stride
        );
        platform->buffer_format_logged = 1;
    }

    app_set_screen_size(&platform->app, (float)buffer.width, (float)buffer.height);
    pthread_mutex_lock(&platform->input_queue_mutex);
    platform_poll_soft_keyboard_text(platform);
    app_update(&platform->app, &platform->input, dt);
    should_show_keyboard = app_take_soft_keyboard_request(&platform->app);
    input_end_frame(&platform->input);
    pthread_mutex_unlock(&platform->input_queue_mutex);
    renderer_draw_frame(&buffer, &platform->app);
    ANativeWindow_unlockAndPost(window);
    ANativeWindow_release(window);

    if (should_show_keyboard && platform->activity != NULL) {
        if (!platform_show_soft_keyboard_java(platform->activity)) {
            ANativeActivity_showSoftInput(platform->activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
        }
    }

    return 1;
}

static int platform_take_reset_frame_time(AndroidPlatform* platform) {
    int reset_frame_time;

    pthread_mutex_lock(&platform->window_mutex);
    reset_frame_time = platform->reset_frame_time;
    platform->reset_frame_time = 0;
    pthread_mutex_unlock(&platform->window_mutex);

    return reset_frame_time;
}

static void platform_update_fps(AndroidPlatform* platform, float frame_time) {
    if (platform == NULL) {
        return;
    }

    platform->fps_elapsed += frame_time;
    platform->fps_frame_time_total += frame_time;
    platform->fps_frame_count += 1;

    if (platform->fps_elapsed < 1.0) {
        return;
    }

    platform->app.fps = (int)((double)platform->fps_frame_count / platform->fps_elapsed + 0.5);
    platform->app.averageFrameMs = (int)((platform->fps_frame_time_total * 1000.0)
            / (double)platform->fps_frame_count);

    platform->fps_elapsed = 0.0;
    platform->fps_frame_time_total = 0.0;
    platform->fps_frame_count = 0;
}

static void platform_handle_exit_requested(AndroidPlatform* platform) {
    if (platform == NULL
            || !platform->app.exitRequested
            || platform->finish_requested) {
        return;
    }

    platform->finish_requested = 1;
    if (platform->activity != NULL) {
        ANativeActivity_finish(platform->activity);
    }
}

static void* platform_game_loop(void* data) {
    AndroidPlatform* platform = (AndroidPlatform*)data;
    const double target_frame_seconds = 1.0 / 60.0;
    const double reset_frame_seconds = 1.0 / 60.0;
    double last_time = platform_now_seconds();

    ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    LOGI("Game loop start");

    while (platform->loop_running) {
        double frame_start = platform_now_seconds();
        float dt = (float)(frame_start - last_time);
        float input_screen_width = (float)platform->app.screenWidth;
        double frame_elapsed;

        last_time = frame_start;
        if (platform_take_reset_frame_time(platform) || dt > 0.1f) {
            dt = (float)reset_frame_seconds;
        }

        platform_process_input(platform, input_screen_width);
        platform_handle_exit_requested(platform);

        if (platform_draw(platform, dt)) {
            platform_update_fps(platform, dt);
        }
        platform_handle_exit_requested(platform);

        frame_elapsed = platform_now_seconds() - frame_start;
        platform_sleep_seconds(target_frame_seconds - frame_elapsed);
    }

    LOGI("Game loop stop");
    return NULL;
}

static void platform_start_game_loop(AndroidPlatform* platform) {
    if (platform == NULL || platform->loop_running) {
        return;
    }

    platform->loop_running = 1;

    if (pthread_create(&platform->thread, NULL, platform_game_loop, platform) != 0) {
        platform->loop_running = 0;
        LOGE("Failed to start game loop");
    }
}

static void platform_stop_game_loop(AndroidPlatform* platform) {
    if (platform == NULL || !platform->loop_running) {
        return;
    }

    platform->loop_running = 0;
    pthread_join(platform->thread, NULL);
}

static void platform_on_input_queue_created(
        ANativeActivity* activity,
        AInputQueue* queue
) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    pthread_mutex_lock(&platform->input_queue_mutex);
    platform->input_queue = queue;
    pthread_mutex_unlock(&platform->input_queue_mutex);
}

static void platform_on_input_queue_destroyed(
        ANativeActivity* activity,
        AInputQueue* queue
) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    pthread_mutex_lock(&platform->input_queue_mutex);
    if (platform->attached_input_queue == queue) {
        AInputQueue_detachLooper(platform->attached_input_queue);
        platform->attached_input_queue = NULL;
    }
    platform->input_queue = NULL;
    platform->input_cancel_requested = 1;
    pthread_mutex_unlock(&platform->input_queue_mutex);
}

static void platform_on_native_window_created(
        ANativeActivity* activity,
        ANativeWindow* window
) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);

    pthread_mutex_lock(&platform->window_mutex);
    platform->window = window;
    platform->null_window_logged = 0;
    platform->reset_frame_time = 1;
    pthread_mutex_unlock(&platform->window_mutex);
}

static void platform_on_native_window_destroyed(
        ANativeActivity* activity,
        ANativeWindow* window
) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    pthread_mutex_lock(&platform->window_mutex);
    if (platform->window == window) {
        platform->window = NULL;
    }
    pthread_mutex_unlock(&platform->window_mutex);
}

static void platform_on_pause(ANativeActivity* activity) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    pthread_mutex_lock(&platform->input_queue_mutex);
    input_handle_touch(&platform->input, INPUT_TOUCH_CANCEL, -1, 0.0f, 0.0f, 0.0f);
    pthread_mutex_unlock(&platform->input_queue_mutex);
    audio_pause();
}

static void platform_on_resume(ANativeActivity* activity) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    pthread_mutex_lock(&platform->window_mutex);
    platform->reset_frame_time = 1;
    pthread_mutex_unlock(&platform->window_mutex);
    audio_resume();
}

static void platform_on_destroy(ANativeActivity* activity) {
    AndroidPlatform* platform = platform_from_activity(activity);
    if (platform == NULL) {
        return;
    }

    platform_stop_game_loop(platform);
    pthread_mutex_lock(&platform->window_mutex);
    platform->window = NULL;
    pthread_mutex_unlock(&platform->window_mutex);
    pthread_mutex_lock(&platform->input_queue_mutex);
    if (platform->attached_input_queue != NULL) {
        AInputQueue_detachLooper(platform->attached_input_queue);
        platform->attached_input_queue = NULL;
    }
    pthread_mutex_unlock(&platform->input_queue_mutex);
    pthread_mutex_destroy(&platform->input_queue_mutex);
    pthread_mutex_destroy(&platform->window_mutex);
    generated_sprite_shutdown_all();
    audio_shutdown();
    activity->instance = NULL;

    free(platform);
}

void platform_android_on_create(
        ANativeActivity* activity,
        void* saved_state,
        size_t saved_state_size
) {
    AndroidPlatform* platform;

    (void)saved_state;
    (void)saved_state_size;

    platform = (AndroidPlatform*)calloc(1, sizeof(AndroidPlatform));
    if (platform == NULL) {
        LOGE("Failed to allocate Android platform state");
        return;
    }

    activity->instance = platform;
    platform->activity = activity;
    pthread_mutex_init(&platform->window_mutex, NULL);
    pthread_mutex_init(&platform->input_queue_mutex, NULL);
    platform->reset_frame_time = 1;

    audio_init();
    audio_bind_android(activity->vm, activity->clazz);
    cat_chess_api_bind_android(activity->vm, activity->clazz);
    generated_sprite_initialize_all();
    local_storage_set_base_path(activity->internalDataPath);
    app_init(&platform->app);
    input_init(&platform->input);

    activity->callbacks->onInputQueueCreated = platform_on_input_queue_created;
    activity->callbacks->onInputQueueDestroyed = platform_on_input_queue_destroyed;
    activity->callbacks->onNativeWindowCreated = platform_on_native_window_created;
    activity->callbacks->onNativeWindowDestroyed = platform_on_native_window_destroyed;
    activity->callbacks->onPause = platform_on_pause;
    activity->callbacks->onResume = platform_on_resume;
    activity->callbacks->onDestroy = platform_on_destroy;

    LOGI("Cat Chess native activity created");
    platform_start_game_loop(platform);
}
