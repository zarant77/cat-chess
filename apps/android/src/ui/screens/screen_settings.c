#include "screen_settings.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../../sprites/generated_sprite.h"
#include "../ui_controls.h"

#define SETTINGS_LANGUAGE_BUTTON_MAX_WIDTH 520
#define SETTINGS_LANGUAGE_BUTTON_HEIGHT 100
#define SETTINGS_LANGUAGE_BUTTON_GAP 24
#define SETTINGS_FLAG_SIZE 68

static UiRect screen_settings_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = 260;
    rect.height = 76;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight - rect.height - 20;
    return rect;
}

static UiRect screen_settings_language_rect(const AppState* app, GameLocale locale) {
    UiRect rect;
    int width = app->screenWidth - 64;
    int start_y = app->screenHeight / 2 - 54;

    if (width > SETTINGS_LANGUAGE_BUTTON_MAX_WIDTH) {
        width = SETTINGS_LANGUAGE_BUTTON_MAX_WIDTH;
    }
    if (width < 280) {
        width = 280;
    }

    rect.width = width;
    rect.height = SETTINGS_LANGUAGE_BUTTON_HEIGHT;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = locale == GAME_LOCALE_UKRAINIAN
            ? start_y
            : start_y + SETTINGS_LANGUAGE_BUTTON_HEIGHT + SETTINGS_LANGUAGE_BUTTON_GAP;
    return rect;
}

static void screen_settings_draw_outline(
        Framebuffer* framebuffer,
        UiRect rect,
        int thickness,
        uint32_t color
) {
    renderer_draw_color_rect(framebuffer, rect.x, rect.y, rect.width, thickness, color);
    renderer_draw_color_rect(framebuffer, rect.x, rect.y + rect.height - thickness, rect.width, thickness, color);
    renderer_draw_color_rect(framebuffer, rect.x, rect.y, thickness, rect.height, color);
    renderer_draw_color_rect(framebuffer, rect.x + rect.width - thickness, rect.y, thickness, rect.height, color);
}

static void screen_settings_draw_language_button(
        Framebuffer* framebuffer,
        const PackedFont* font,
        const AppState* app,
        GameLocale locale,
        const char* sprite_id,
        const char* label
) {
    UiRect rect = screen_settings_language_rect(app, locale);
    const GeneratedSprite* sprite = generated_sprite_get_by_id(sprite_id);
    int selected = game_settings_normalize_locale(app->settings.locale) == locale;
    int flag_x = rect.x + 20;
    int flag_y = rect.y + (rect.height - SETTINGS_FLAG_SIZE) / 2;
    int text_width = font_measure_text(font, 3, label);
    int text_x = rect.x + 128 + (rect.width - 128 - text_width) / 2;
    int text_y = rect.y + (rect.height - (int)font->grid_size * 3) / 2;

    renderer_draw_color_rect(framebuffer, rect.x, rect.y, rect.width, rect.height, selected ? 0x4a5f55ffu : 0x3d405bffu);
    screen_settings_draw_outline(framebuffer, rect, selected ? 6 : 3, selected ? 0xf4c542ffu : 0x81b29affu);

    renderer_draw_generated_sprite_fit(
            framebuffer,
            sprite,
            flag_x,
            flag_y,
            SETTINGS_FLAG_SIZE,
            SETTINGS_FLAG_SIZE,
            SPRITE_FIT_CONTAIN
    );
    ui_draw_label(framebuffer, font, text_x, text_y, 3, 0xffffffffu, label);
}

void screen_settings_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_SETTINGS);
    const char* language = localization_text(app->settings.locale, LOCALIZED_TEXT_LANGUAGE);
    int title_width = font_measure_text(font, 4, title);
    int language_width = font_measure_text(font, 3, language);

    ui_draw_label(framebuffer, font, (app->screenWidth - title_width) / 2, app->screenHeight / 2 - 170, 4, 0x27312bffu, title);
    ui_draw_label(framebuffer, font, (app->screenWidth - language_width) / 2, app->screenHeight / 2 - 72, 3, 0x3d405bffu, language);
    screen_settings_draw_language_button(
            framebuffer,
            font,
            app,
            GAME_LOCALE_UKRAINIAN,
            "flag_uk",
            localization_text(app->settings.locale, LOCALIZED_TEXT_LANGUAGE_UKRAINIAN)
    );
    screen_settings_draw_language_button(
            framebuffer,
            font,
            app,
            GAME_LOCALE_ENGLISH,
            "flag_en",
            localization_text(app->settings.locale, LOCALIZED_TEXT_LANGUAGE_ENGLISH)
    );
    ui_draw_button_scaled(framebuffer, font, screen_settings_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), 3);
}

void screen_settings_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect uk_rect;
    UiRect en_rect;
    if (app == 0) {
        return;
    }
    back_rect = screen_settings_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
        return;
    }

    uk_rect = screen_settings_language_rect(app, GAME_LOCALE_UKRAINIAN);
    if (ui_rect_contains(&uk_rect, x, y)) {
        game_settings_set_locale(&app->settings, GAME_LOCALE_UKRAINIAN);
        return;
    }

    en_rect = screen_settings_language_rect(app, GAME_LOCALE_ENGLISH);
    if (ui_rect_contains(&en_rect, x, y)) {
        game_settings_set_locale(&app->settings, GAME_LOCALE_ENGLISH);
    }
}
