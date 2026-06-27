#ifndef CAT_CHESS_GENERATED_MUSIC_H
#define CAT_CHESS_GENERATED_MUSIC_H

#include <stddef.h>
#include <stdint.h>

#include "music_definition.h"

typedef enum
{
    MUSIC_ID_NONE = -1,
    MUSIC_ID_CHESS_THEME = 0,
    MUSIC_ID_COUNT
} MusicId;

#define MUSIC_NONE MUSIC_ID_NONE
#define MUSIC_CHESS_THEME MUSIC_ID_CHESS_THEME

typedef struct
{
    const char *id;
    int32_t sample_rate;
    int32_t sample_count;
    int32_t loop_enabled;
    int32_t loop_start_sample;
    int32_t loop_end_sample;
    int32_t volume;
    int16_t *samples;
} GeneratedMusic;

extern const PackedMusicDefinition PACKED_MUSIC_DEFINITIONS[];
extern const size_t PACKED_MUSIC_COUNT;

const char* generated_music_id_name(MusicId music_id);

#endif
