#include "screen_settings.h"

#ifdef __ANDROID__
#include <android/log.h>
#include "../../config.h"
#define SETTINGS_LOG(...) __android_log_print(ANDROID_LOG_WARN, CAT_CHESS_LOG_TAG, __VA_ARGS__)
#else
#define SETTINGS_LOG(...) ((void)0)
#endif

#include "../../audio/chess_sound.h"
#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static UiRect screen_settings_back_rect(const AppState* app);

static int screen_settings_title_scale(const AppState* app) {
    if (app->screenHeight < 600) {
        return 2;
    }
    return app->screenWidth < 420 ? 3 : 4;
}

static int screen_settings_toggle_height(const AppState* app) {
    if (app->screenHeight < 520) {
        return 42;
    }
    return app->screenHeight < 640 ? 56 : 66;
}

static int screen_settings_language_height(const AppState* app) {
    if (app->screenHeight < 520) {
        return 42;
    }
    return app->screenHeight < 640 ? 58 : 72;
}

static int screen_settings_difficulty_height(const AppState* app) {
    if (app->screenHeight < 520) {
        return 38;
    }
    return app->screenHeight < 640 ? 52 : 58;
}

static int screen_settings_title_gap(const AppState* app) {
    return app->screenHeight < 520 ? UI_SPACE_XS : UI_SPACE_LG;
}

static int screen_settings_row_gap(const AppState* app) {
    return app->screenHeight < 520 ? UI_SPACE_XS : UI_SPACE_MD;
}

static int screen_settings_language_gap(const AppState* app) {
    return app->screenHeight < 520 ? UI_SPACE_XS : (app->screenHeight < 640 ? UI_SPACE_SM : UI_SPACE_MD);
}

static int screen_settings_language_label_height(const AppState* app) {
    return app->screenHeight < 520 ? 20 : 28;
}

static int screen_settings_difficulty_label_height(const AppState* app) {
    return app->screenHeight < 520 ? 20 : 28;
}

static int screen_settings_group_height(const AppState* app) {
    return screen_settings_title_scale(app) * 20
            + screen_settings_title_gap(app)
            + screen_settings_toggle_height(app) * 3
            + screen_settings_row_gap(app) * 3
            + screen_settings_difficulty_label_height(app)
            + screen_settings_difficulty_height(app)
            + screen_settings_language_gap(app)
            + screen_settings_language_gap(app)
            + screen_settings_language_label_height(app)
            + screen_settings_language_height(app);
}

static int screen_settings_group_y(const AppState* app) {
    int top = ui_top_safe_padding(app->screenHeight);
    int bottom = screen_settings_back_rect(app).y - UI_SPACE_MD;
    int available = bottom - top;
    int height = screen_settings_group_height(app);

    if (available < height) {
        return top;
    }

    return top + (available - height) / 2;
}

static UiRect screen_settings_content_rect(const AppState* app) {
    UiRect rect;

    rect.width = ui_content_width(app->screenWidth);
    rect.height = screen_settings_group_height(app);
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = screen_settings_group_y(app) + screen_settings_title_scale(app) * 20 + screen_settings_title_gap(app);
    return rect;
}

static UiRect screen_settings_toggle_rect(const AppState* app, int index) {
    UiRect content = screen_settings_content_rect(app);
    UiRect rect;

    rect.width = content.width;
    rect.height = screen_settings_toggle_height(app);
    rect.x = content.x;
    rect.y = content.y + index * (rect.height + screen_settings_row_gap(app));
    return rect;
}

static UiRect screen_settings_language_rect(const AppState* app, GameLocale locale) {
    UiRect content = screen_settings_content_rect(app);
    UiRect rect;
    int gap = UI_SPACE_MD;
    int toggle_height = screen_settings_toggle_height(app);
    int difficulty_label_y = content.y + toggle_height * 3 + screen_settings_row_gap(app) * 3;
    int difficulty_y = difficulty_label_y + screen_settings_difficulty_label_height(app);
    int label_y = difficulty_y
            + screen_settings_difficulty_height(app)
            + screen_settings_language_gap(app);

    rect.width = (content.width - gap) / 2;
    rect.height = screen_settings_language_height(app);
    rect.x = content.x + (locale == GAME_LOCALE_ENGLISH ? rect.width + gap : 0);
    rect.y = label_y + screen_settings_language_label_height(app);
    return rect;
}

static UiRect screen_settings_difficulty_rect(const AppState* app, ChessAiDifficulty difficulty) {
    UiRect content = screen_settings_content_rect(app);
    UiRect rect;
    int gap = UI_SPACE_SM;
    int toggle_height = screen_settings_toggle_height(app);
    int label_y = content.y + toggle_height * 3 + screen_settings_row_gap(app) * 3;
    int index = 1;

    if (difficulty == CHESS_AI_EASY) {
        index = 0;
    } else if (difficulty == CHESS_AI_HARD) {
        index = 2;
    }

    rect.width = (content.width - gap * 2) / 3;
    rect.height = screen_settings_difficulty_height(app);
    rect.x = content.x + index * (rect.width + gap);
    rect.y = label_y + screen_settings_difficulty_label_height(app);
    return rect;
}

static UiRect screen_settings_back_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT,
            width,
            UI_BUTTON_HEIGHT_COMPACT
    );
}

static void screen_settings_draw_checkbox(Framebuffer* framebuffer, UiRect rect, int checked) {
    UiRect box;
    uint32_t fill = checked ? 0x4a665affu : 0xf9f1e1ffu;
    uint32_t border = checked ? UI_COLOR_ACCENT : UI_COLOR_TEXT;
    int mark;

    box.width = rect.height < 54 ? rect.height - 12 : 44;
    box.height = box.width;
    box.x = rect.x + UI_SPACE_MD;
    box.y = rect.y + (rect.height - box.height) / 2;

    renderer_draw_color_rect(framebuffer, box.x, box.y, box.width, box.height, fill);
    renderer_draw_color_rect(framebuffer, box.x, box.y, box.width, 4, border);
    renderer_draw_color_rect(framebuffer, box.x, box.y + box.height - 4, box.width, 4, border);
    renderer_draw_color_rect(framebuffer, box.x, box.y, 4, box.height, border);
    renderer_draw_color_rect(framebuffer, box.x + box.width - 4, box.y, 4, box.height, border);

    if (!checked) {
        return;
    }

    if (box.width < 38) {
        for (mark = 0; mark < 3; ++mark) {
            renderer_draw_color_rect(framebuffer, box.x + 8 + mark * 4, box.y + 17 + mark * 2, 5, 5, UI_COLOR_TEXT_ON_DARK);
            renderer_draw_color_rect(framebuffer, box.x + 19 + mark * 3, box.y + 21 - mark * 4, 5, 5, UI_COLOR_TEXT_ON_DARK);
        }
        return;
    }

    for (mark = 0; mark < 5; ++mark) {
        renderer_draw_color_rect(framebuffer, box.x + 12 + mark * 4, box.y + 25 + mark * 2, 6, 6, UI_COLOR_TEXT_ON_DARK);
        renderer_draw_color_rect(framebuffer, box.x + 27 + mark * 4, box.y + 30 - mark * 5, 6, 6, UI_COLOR_TEXT_ON_DARK);
    }
}

static void screen_settings_draw_toggle(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        int checked
) {
    int scale = 2;
    int text_x = rect.x + UI_SPACE_MD + 62;
    int text_y;

    renderer_draw_color_rect(framebuffer, rect.x + 3, rect.y + 4, rect.width, rect.height, 0x26324422u);
    renderer_draw_color_rect(framebuffer, rect.x, rect.y, rect.width, rect.height, 0xf9f1e1aau);
    screen_settings_draw_checkbox(framebuffer, rect, checked);
    if (font == 0 || label == 0) {
        return;
    }
    text_y = rect.y + (rect.height - (int)font->grid_size * scale) / 2;
    ui_draw_label(framebuffer, font, text_x, text_y, scale, UI_COLOR_TEXT, label);
}

static void screen_settings_draw_language_button(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* sprite_id,
        int selected
) {
    const GeneratedSprite* sprite = generated_sprite_get_by_id(sprite_id);
    UiRect sprite_rect;

    ui_draw_button_style(framebuffer, font, rect, 0, selected ? UI_BUTTON_SELECTED : UI_BUTTON_SECONDARY, 2);
    if (sprite == 0) {
        SETTINGS_LOG("Missing language flag sprite: %s", sprite_id != 0 ? sprite_id : "(null)");
        return;
    }

    sprite_rect.width = ui_min_int(rect.width - UI_SPACE_SM * 2, 118);
    sprite_rect.height = (sprite_rect.width * 64) / 96;
    if (sprite_rect.height > rect.height - UI_SPACE_SM * 2) {
        sprite_rect.height = rect.height - UI_SPACE_SM * 2;
        sprite_rect.width = (sprite_rect.height * 96) / 64;
    }
    sprite_rect.x = rect.x + (rect.width - sprite_rect.width) / 2;
    sprite_rect.y = rect.y + (rect.height - sprite_rect.height) / 2;
    renderer_draw_generated_sprite_fit(framebuffer, sprite, sprite_rect.x, sprite_rect.y, sprite_rect.width, sprite_rect.height, SPRITE_FIT_CONTAIN);
}

void screen_settings_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    int title_scale = screen_settings_title_scale(app);
    int selected_uk = game_settings_normalize_locale(app->settings.locale) == GAME_LOCALE_UKRAINIAN;
    int selected_en = game_settings_normalize_locale(app->settings.locale) == GAME_LOCALE_ENGLISH;
    ChessAiDifficulty selected_difficulty = game_settings_normalize_ai_difficulty(app->settings.ai_difficulty);
    UiRect content = screen_settings_content_rect(app);
    int toggle_height = screen_settings_toggle_height(app);
    int difficulty_label_y = content.y + toggle_height * 3 + screen_settings_row_gap(app) * 3;
    int difficulty_y = difficulty_label_y + screen_settings_difficulty_label_height(app);
    int language_label_y = difficulty_y
            + screen_settings_difficulty_height(app)
            + screen_settings_language_gap(app);
    int group_y = screen_settings_group_y(app);
    int title_y = group_y;

    ui_draw_centered_label(framebuffer, font, app->screenWidth, title_y, title_scale, UI_COLOR_TEXT, localization_text(app->settings.locale, LOCALIZED_TEXT_SETTINGS));

    screen_settings_draw_toggle(
            framebuffer,
            font,
            screen_settings_toggle_rect(app, 0),
            localization_text(app->settings.locale, LOCALIZED_TEXT_SFX),
            app->settings.sounds_enabled
    );
    screen_settings_draw_toggle(
            framebuffer,
            font,
            screen_settings_toggle_rect(app, 1),
            localization_text(app->settings.locale, LOCALIZED_TEXT_MUSIC),
            app->settings.music_enabled
    );
    screen_settings_draw_toggle(
            framebuffer,
            font,
            screen_settings_toggle_rect(app, 2),
            localization_text(app->settings.locale, LOCALIZED_TEXT_HINTS),
            app->settings.show_move_hints
    );

    ui_draw_label(framebuffer, font, content.x, difficulty_label_y, 2, UI_COLOR_TEXT, localization_text(app->settings.locale, LOCALIZED_TEXT_AI_DIFFICULTY));
    ui_draw_button_style(
            framebuffer,
            font,
            screen_settings_difficulty_rect(app, CHESS_AI_EASY),
            localization_text(app->settings.locale, LOCALIZED_TEXT_AI_EASY),
            selected_difficulty == CHESS_AI_EASY ? UI_BUTTON_SELECTED : UI_BUTTON_SECONDARY,
            1
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_settings_difficulty_rect(app, CHESS_AI_NORMAL),
            localization_text(app->settings.locale, LOCALIZED_TEXT_AI_NORMAL),
            selected_difficulty == CHESS_AI_NORMAL ? UI_BUTTON_SELECTED : UI_BUTTON_SECONDARY,
            1
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_settings_difficulty_rect(app, CHESS_AI_HARD),
            localization_text(app->settings.locale, LOCALIZED_TEXT_AI_HARD),
            selected_difficulty == CHESS_AI_HARD ? UI_BUTTON_SELECTED : UI_BUTTON_SECONDARY,
            1
    );

    ui_draw_label(framebuffer, font, content.x, language_label_y, 2, UI_COLOR_TEXT, localization_text(app->settings.locale, LOCALIZED_TEXT_LANGUAGE));
    screen_settings_draw_language_button(
            framebuffer,
            font,
            screen_settings_language_rect(app, GAME_LOCALE_UKRAINIAN),
            "flag_uk",
            selected_uk
    );
    screen_settings_draw_language_button(
            framebuffer,
            font,
            screen_settings_language_rect(app, GAME_LOCALE_ENGLISH),
            "flag_en",
            selected_en
    );

    ui_draw_button_style(framebuffer, font, screen_settings_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), UI_BUTTON_COMPACT, 2);
}

void screen_settings_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect sounds_rect;
    UiRect music_rect;
    UiRect hints_rect;
    UiRect easy_rect;
    UiRect normal_rect;
    UiRect hard_rect;
    UiRect uk_rect;
    UiRect en_rect;

    if (app == 0) {
        return;
    }

    back_rect = screen_settings_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, app->settingsReturnScreen);
        return;
    }

    sounds_rect = screen_settings_toggle_rect(app, 0);
    if (ui_rect_contains(&sounds_rect, x, y)) {
        game_settings_set_sounds_enabled(&app->settings, !app->settings.sounds_enabled);
        app_save_settings(app);
        if (app->settings.sounds_enabled) {
            chess_sound_play(CHESS_SOUND_MENU_SELECT);
        }
        return;
    }

    music_rect = screen_settings_toggle_rect(app, 1);
    if (ui_rect_contains(&music_rect, x, y)) {
        game_settings_set_music_enabled(&app->settings, !app->settings.music_enabled);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
        return;
    }

    hints_rect = screen_settings_toggle_rect(app, 2);
    if (ui_rect_contains(&hints_rect, x, y)) {
        game_settings_set_show_move_hints(&app->settings, !app->settings.show_move_hints);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
        return;
    }

    easy_rect = screen_settings_difficulty_rect(app, CHESS_AI_EASY);
    if (ui_rect_contains(&easy_rect, x, y)) {
        game_settings_set_ai_difficulty(&app->settings, CHESS_AI_EASY);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
        return;
    }

    normal_rect = screen_settings_difficulty_rect(app, CHESS_AI_NORMAL);
    if (ui_rect_contains(&normal_rect, x, y)) {
        game_settings_set_ai_difficulty(&app->settings, CHESS_AI_NORMAL);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
        return;
    }

    hard_rect = screen_settings_difficulty_rect(app, CHESS_AI_HARD);
    if (ui_rect_contains(&hard_rect, x, y)) {
        game_settings_set_ai_difficulty(&app->settings, CHESS_AI_HARD);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
        return;
    }

    uk_rect = screen_settings_language_rect(app, GAME_LOCALE_UKRAINIAN);
    if (ui_rect_contains(&uk_rect, x, y)) {
        game_settings_set_locale(&app->settings, GAME_LOCALE_UKRAINIAN);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
        return;
    }

    en_rect = screen_settings_language_rect(app, GAME_LOCALE_ENGLISH);
    if (ui_rect_contains(&en_rect, x, y)) {
        game_settings_set_locale(&app->settings, GAME_LOCALE_ENGLISH);
        app_save_settings(app);
        chess_sound_play(CHESS_SOUND_MENU_SELECT);
    }
}
