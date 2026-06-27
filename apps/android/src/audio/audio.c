#include "audio.h"

#include <stdint.h>

#include "music_registry.h"

static int audio_sfx_volume = 80;
static int audio_sounds_enabled = 1;
static int audio_music_enabled = 1;
static JavaVM* audio_java_vm;
static jclass audio_class;
static jmethodID audio_play_method;
static jmethodID audio_play_music_pcm_method;
static jmethodID audio_stop_music_method;

static int audio_clamp_volume(int volume) {
    if (volume < 0) {
        return 0;
    }
    if (volume > 100) {
        return 100;
    }
    return volume;
}

static JNIEnv* audio_get_env(int* did_attach) {
    JNIEnv* env = 0;

    if (did_attach != 0) {
        *did_attach = 0;
    }
    if (audio_java_vm == 0) {
        return 0;
    }
    if ((*audio_java_vm)->GetEnv(audio_java_vm, (void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        return env;
    }
    if ((*audio_java_vm)->AttachCurrentThread(audio_java_vm, &env, 0) != JNI_OK) {
        return 0;
    }
    if (did_attach != 0) {
        *did_attach = 1;
    }
    return env;
}

static void audio_clear_exception(JNIEnv* env) {
    if (env != 0 && (*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }
}

static jclass audio_load_class(JNIEnv* env, jobject activity, const char* class_name_value) {
    jclass activity_class;
    jmethodID get_class_loader_method;
    jobject class_loader;
    jclass class_loader_class;
    jmethodID load_class_method;
    jstring class_name;
    jclass loaded_class = 0;

    if (env == 0 || activity == 0 || class_name_value == 0) {
        return 0;
    }

    activity_class = (*env)->GetObjectClass(env, activity);
    if (activity_class == 0) {
        audio_clear_exception(env);
        return 0;
    }

    get_class_loader_method = (*env)->GetMethodID(env, activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
    if (get_class_loader_method == 0) {
        audio_clear_exception(env);
        (*env)->DeleteLocalRef(env, activity_class);
        return 0;
    }

    class_loader = (*env)->CallObjectMethod(env, activity, get_class_loader_method);
    (*env)->DeleteLocalRef(env, activity_class);
    if (class_loader == 0 || (*env)->ExceptionCheck(env)) {
        audio_clear_exception(env);
        return 0;
    }

    class_loader_class = (*env)->FindClass(env, "java/lang/ClassLoader");
    if (class_loader_class == 0) {
        audio_clear_exception(env);
        (*env)->DeleteLocalRef(env, class_loader);
        return 0;
    }

    load_class_method = (*env)->GetMethodID(env, class_loader_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (load_class_method == 0) {
        audio_clear_exception(env);
        (*env)->DeleteLocalRef(env, class_loader_class);
        (*env)->DeleteLocalRef(env, class_loader);
        return 0;
    }

    class_name = (*env)->NewStringUTF(env, class_name_value);
    if (class_name != 0) {
        loaded_class = (jclass)(*env)->CallObjectMethod(env, class_loader, load_class_method, class_name);
        (*env)->DeleteLocalRef(env, class_name);
        if ((*env)->ExceptionCheck(env)) {
            audio_clear_exception(env);
            loaded_class = 0;
        }
    }

    (*env)->DeleteLocalRef(env, class_loader_class);
    (*env)->DeleteLocalRef(env, class_loader);
    return loaded_class;
}

void audio_init(void) {
    audio_sfx_volume = 80;
    audio_sounds_enabled = 1;
    audio_music_enabled = 1;
    music_registry_initialize_all();
}

void audio_bind_android(JavaVM* vm, jobject activity) {
    JNIEnv* env = 0;
    jclass local_class;

    audio_java_vm = vm;
    if (vm == 0 || activity == 0) {
        return;
    }
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK || env == 0) {
        return;
    }

    local_class = audio_load_class(env, activity, "com.catemup.catchess.CatChessAudio");
    if (local_class == 0) {
        return;
    }

    audio_class = (jclass)(*env)->NewGlobalRef(env, local_class);
    (*env)->DeleteLocalRef(env, local_class);
    if (audio_class == 0) {
        return;
    }

    audio_play_method = (*env)->GetStaticMethodID(env, audio_class, "play", "(Ljava/lang/String;I)V");
    audio_play_music_pcm_method = (*env)->GetStaticMethodID(env, audio_class, "playMusicPcm", "(Ljava/lang/String;[SIIII)V");
    audio_stop_music_method = (*env)->GetStaticMethodID(env, audio_class, "stopMusic", "()V");
    audio_clear_exception(env);
}

void audio_shutdown(void) {
    JNIEnv* env;
    int did_attach = 0;

    audio_stop_music();
    if (audio_class != 0) {
        env = audio_get_env(&did_attach);
        if (env != 0) {
            (*env)->DeleteGlobalRef(env, audio_class);
        }
        if (did_attach && audio_java_vm != 0) {
            (*audio_java_vm)->DetachCurrentThread(audio_java_vm);
        }
    }
    audio_class = 0;
    audio_play_method = 0;
    audio_play_music_pcm_method = 0;
    audio_stop_music_method = 0;
    audio_java_vm = 0;
    music_registry_shutdown_all();
}

void audio_pause(void) {
    audio_stop_music();
}

void audio_resume(void) {
    if (audio_music_enabled) {
        audio_play_music("chess_theme");
    }
}

void audio_set_sfx_volume(int volume) {
    audio_sfx_volume = audio_clamp_volume(volume);
}

void audio_set_sounds_enabled(int enabled) {
    audio_sounds_enabled = enabled ? 1 : 0;
}

int audio_are_sounds_enabled(void) {
    return audio_sounds_enabled;
}

void audio_set_music_enabled(int enabled) {
    audio_music_enabled = enabled ? 1 : 0;
    if (!audio_music_enabled) {
        audio_stop_music();
    }
}

int audio_is_music_enabled(void) {
    return audio_music_enabled;
}

void audio_play_music(const char* id) {
    JNIEnv* env;
    const GeneratedMusic* music;
    jstring music_id;
    jshortArray music_samples;
    int loop_start;
    int loop_end;
    int did_attach = 0;

    if (!audio_music_enabled || id == 0 || id[0] == '\0') {
        return;
    }
    if (audio_java_vm == 0 || audio_class == 0 || audio_play_music_pcm_method == 0) {
        return;
    }

    music = music_registry_get_by_id(id);
    if (music == 0 || music->samples == 0 || music->sample_count <= 0 || music->sample_rate <= 0) {
        return;
    }

    env = audio_get_env(&did_attach);
    if (env == 0) {
        return;
    }

    music_id = (*env)->NewStringUTF(env, id);
    music_samples = (*env)->NewShortArray(env, (jsize)music->sample_count);
    if (music_id != 0 && music_samples != 0) {
        loop_start = music->loop_enabled ? music->loop_start_sample : 0;
        loop_end = music->loop_enabled ? music->loop_end_sample : music->sample_count;
        if (loop_start < 0 || loop_start >= music->sample_count) {
            loop_start = 0;
        }
        if (loop_end <= loop_start || loop_end > music->sample_count) {
            loop_end = music->sample_count;
        }

        (*env)->SetShortArrayRegion(env, music_samples, 0, (jsize)music->sample_count, (const jshort*)music->samples);
        if (!(*env)->ExceptionCheck(env)) {
            (*env)->CallStaticVoidMethod(
                env,
                audio_class,
                audio_play_music_pcm_method,
                music_id,
                music_samples,
                music->sample_rate,
                loop_start,
                loop_end,
                25);
        }
    }
    if (music_samples != 0) {
        (*env)->DeleteLocalRef(env, music_samples);
    }
    if (music_id != 0) {
        (*env)->DeleteLocalRef(env, music_id);
    }
    audio_clear_exception(env);

    if (did_attach && audio_java_vm != 0) {
        (*audio_java_vm)->DetachCurrentThread(audio_java_vm);
    }
}

void audio_stop_music(void) {
    JNIEnv* env;
    int did_attach = 0;

    if (audio_java_vm == 0 || audio_class == 0 || audio_stop_music_method == 0) {
        return;
    }

    env = audio_get_env(&did_attach);
    if (env == 0) {
        return;
    }

    (*env)->CallStaticVoidMethod(env, audio_class, audio_stop_music_method);
    audio_clear_exception(env);

    if (did_attach && audio_java_vm != 0) {
        (*audio_java_vm)->DetachCurrentThread(audio_java_vm);
    }
}

void audio_play_sound(const char* id) {
    JNIEnv* env;
    jstring sound_id;
    int did_attach = 0;

    if (!audio_sounds_enabled || audio_sfx_volume <= 0 || id == 0 || id[0] == '\0') {
        return;
    }
    if (audio_java_vm == 0 || audio_class == 0 || audio_play_method == 0) {
        return;
    }

    env = audio_get_env(&did_attach);
    if (env == 0) {
        return;
    }

    sound_id = (*env)->NewStringUTF(env, id);
    if (sound_id != 0) {
        (*env)->CallStaticVoidMethod(env, audio_class, audio_play_method, sound_id, audio_sfx_volume);
        (*env)->DeleteLocalRef(env, sound_id);
    }
    audio_clear_exception(env);

    if (did_attach && audio_java_vm != 0) {
        (*audio_java_vm)->DetachCurrentThread(audio_java_vm);
    }
}

void audio_play_ui_sound(const char* id) {
    audio_play_sound(id);
}

void audio_play_game_sound(const char* id) {
    audio_play_sound(id);
}
