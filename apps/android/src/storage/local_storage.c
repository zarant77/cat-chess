#include "local_storage.h"

#include <stdio.h>
#include <string.h>

#define LOCAL_STORAGE_PATH_MAX 512
#define LOCAL_STORAGE_DEVICE_SECRET_FILE "device_secret.txt"
#define LOCAL_STORAGE_SETTINGS_FILE "cat_chess_settings.bin"
#define LOCAL_STORAGE_SETTINGS_VERSION 4u

typedef struct {
    unsigned int version;
    unsigned char sounds_enabled;
    unsigned char music_enabled;
    unsigned char show_move_hints;
    unsigned char ai_difficulty;
    unsigned char locale;
    unsigned char reserved[3];
} LocalStorageSettingsRecord;

typedef struct {
    unsigned int version;
    unsigned char sounds_enabled;
    unsigned char music_enabled;
    unsigned char show_move_hints;
    unsigned char locale;
    unsigned char reserved[4];
} LocalStorageSettingsRecordV3;

typedef struct {
    unsigned int version;
    unsigned char sounds_enabled;
    unsigned char music_enabled;
    unsigned char locale;
    unsigned char reserved[5];
} LocalStorageSettingsRecordV2;

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

static int local_storage_settings_path(char* buffer, int buffer_size) {
    int written;

    if (buffer == 0 || buffer_size <= 0 || local_storage_base_path[0] == '\0') {
        return 0;
    }

    written = snprintf(
            buffer,
            (size_t)buffer_size,
            "%s/%s",
            local_storage_base_path,
            LOCAL_STORAGE_SETTINGS_FILE
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

bool local_storage_get_settings(GameSettings* settings) {
    char path[LOCAL_STORAGE_PATH_MAX];
    FILE* file;
    LocalStorageSettingsRecord record;
    LocalStorageSettingsRecordV3 record_v3;
    LocalStorageSettingsRecordV2 record_v2;
    size_t count;

    if (settings == 0 || !local_storage_settings_path(path, sizeof(path))) {
        return false;
    }

    file = fopen(path, "rb");
    if (file == 0) {
        return false;
    }

    count = fread(&record, 1u, sizeof(record), file);
    fclose(file);

    if (count != sizeof(record)) {
        return false;
    }

    game_settings_init(settings);
    if (record.version == 1u) {
        game_settings_set_locale(settings, game_settings_normalize_locale((int)record.music_enabled));
        game_settings_set_sounds_enabled(settings, record.sounds_enabled ? 1 : 0);
        game_settings_set_music_enabled(settings, 1);
        game_settings_set_show_move_hints(settings, 1);
        game_settings_set_ai_difficulty(settings, CHESS_AI_NORMAL);
        local_storage_set_settings(settings);
        return true;
    }
    if (record.version == 2u) {
        memcpy(&record_v2, &record, sizeof(record_v2));
        game_settings_set_locale(settings, game_settings_normalize_locale((int)record_v2.locale));
        game_settings_set_sounds_enabled(settings, record_v2.sounds_enabled ? 1 : 0);
        game_settings_set_music_enabled(settings, record_v2.music_enabled ? 1 : 0);
        game_settings_set_show_move_hints(settings, 1);
        game_settings_set_ai_difficulty(settings, CHESS_AI_NORMAL);
        local_storage_set_settings(settings);
        return true;
    }
    if (record.version == 3u) {
        memcpy(&record_v3, &record, sizeof(record_v3));
        game_settings_set_locale(settings, game_settings_normalize_locale((int)record_v3.locale));
        game_settings_set_sounds_enabled(settings, record_v3.sounds_enabled ? 1 : 0);
        game_settings_set_music_enabled(settings, record_v3.music_enabled ? 1 : 0);
        game_settings_set_show_move_hints(settings, record_v3.show_move_hints ? 1 : 0);
        game_settings_set_ai_difficulty(settings, CHESS_AI_NORMAL);
        local_storage_set_settings(settings);
        return true;
    }
    if (record.version != LOCAL_STORAGE_SETTINGS_VERSION) {
        return false;
    }

    game_settings_set_locale(settings, game_settings_normalize_locale((int)record.locale));
    game_settings_set_sounds_enabled(settings, record.sounds_enabled ? 1 : 0);
    game_settings_set_music_enabled(settings, record.music_enabled ? 1 : 0);
    game_settings_set_show_move_hints(settings, record.show_move_hints ? 1 : 0);
    game_settings_set_ai_difficulty(settings, game_settings_normalize_ai_difficulty((int)record.ai_difficulty));
    return true;
}

bool local_storage_set_settings(const GameSettings* settings) {
    char path[LOCAL_STORAGE_PATH_MAX];
    FILE* file;
    LocalStorageSettingsRecord record;

    if (settings == 0 || !local_storage_settings_path(path, sizeof(path))) {
        return false;
    }

    record.version = LOCAL_STORAGE_SETTINGS_VERSION;
    record.sounds_enabled = settings->sounds_enabled ? 1u : 0u;
    record.music_enabled = settings->music_enabled ? 1u : 0u;
    record.show_move_hints = settings->show_move_hints ? 1u : 0u;
    record.ai_difficulty = (unsigned char)game_settings_normalize_ai_difficulty(settings->ai_difficulty);
    record.locale = (unsigned char)game_settings_normalize_locale(settings->locale);
    memset(record.reserved, 0, sizeof(record.reserved));

    file = fopen(path, "wb");
    if (file == 0) {
        return false;
    }

    fwrite(&record, 1u, sizeof(record), file);
    fclose(file);
    return true;
}
