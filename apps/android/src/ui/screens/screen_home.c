#include "screen_home.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

#define HOME_BUTTON_COUNT 5
#define HOME_BUTTON_FONT_SCALE 3
#define HOME_BUTTON_HEIGHT 112
#define HOME_BUTTON_GAP 22
#define HOME_TITLE_MIN_Y 36
#define HOME_TITLE_MAX_Y 72
#define HOME_TITLE_BUTTON_GAP 48
#define HOME_BOTTOM_MARGIN 20

static int screen_home_clamp_int(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static int screen_home_title_scale(const AppState* app) {
    return app->screenWidth < 420 ? 3 : 4;
}

static int screen_home_title_y(const AppState* app) {
    return screen_home_clamp_int(app->screenHeight / 12, HOME_TITLE_MIN_Y, HOME_TITLE_MAX_Y);
}

static int screen_home_buttons_start_y(const AppState* app) {
    int title_scale = screen_home_title_scale(app);
    int title_height = 16 * title_scale;
    int min_start_y = screen_home_title_y(app) + title_height + HOME_TITLE_BUTTON_GAP;
    int total_height = HOME_BUTTON_COUNT * HOME_BUTTON_HEIGHT
            + (HOME_BUTTON_COUNT - 1) * HOME_BUTTON_GAP;
    int available_height = app->screenHeight - min_start_y - HOME_BOTTOM_MARGIN;

    if (available_height > total_height) {
        return min_start_y + (available_height - total_height) / 2;
    }

    return min_start_y;
}

static UiRect screen_home_button_rect(const AppState* app, int index) {
    int width = app->screenWidth - 64;
    int max_width = 760;
    int start_y = screen_home_buttons_start_y(app);
    UiRect rect;

    if (width > max_width) {
        width = max_width;
    }
    if (width < 280) {
        width = 280;
    }

    rect.x = (app->screenWidth - width) / 2;
    rect.y = start_y + index * (HOME_BUTTON_HEIGHT + HOME_BUTTON_GAP);
    rect.width = width;
    rect.height = HOME_BUTTON_HEIGHT;
    return rect;
}

void screen_home_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const LocalizedTextId label_ids[HOME_BUTTON_COUNT] = {
            LOCALIZED_TEXT_LOCAL_GAME,
            LOCALIZED_TEXT_CREATE_ONLINE_GAME,
            LOCALIZED_TEXT_JOIN_ONLINE_GAME,
            LOCALIZED_TEXT_MY_GAMES,
            LOCALIZED_TEXT_SETTINGS
    };
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_APP_NAME);
    int title_scale = screen_home_title_scale(app);
    int title_width = font_measure_text(font, title_scale, title);
    int title_y = screen_home_title_y(app);

    ui_draw_label(
            framebuffer,
            font,
            (app->screenWidth - title_width) / 2,
            title_y,
            title_scale,
            0x27312bffu,
            title
    );

    for (int index = 0; index < HOME_BUTTON_COUNT; ++index) {
        ui_draw_button_scaled(
                framebuffer,
                font,
                screen_home_button_rect(app, index),
                localization_text(app->settings.locale, label_ids[index]),
                HOME_BUTTON_FONT_SCALE
        );
    }
}

void screen_home_handle_tap(AppState* app, int x, int y) {
    if (app == 0) {
        return;
    }

    for (int index = 0; index < HOME_BUTTON_COUNT; ++index) {
        UiRect rect = screen_home_button_rect(app, index);
        if (!ui_rect_contains(&rect, x, y)) {
            continue;
        }

        if (index == 0) {
            app_navigate(app, APP_SCREEN_LOCAL_GAME);
        } else if (index == 1) {
            app_navigate(app, APP_SCREEN_CREATE_GAME);
        } else if (index == 2) {
            app_navigate(app, APP_SCREEN_JOIN_GAME);
        } else if (index == 3) {
            app_navigate(app, APP_SCREEN_MY_GAMES);
        } else {
            app_navigate(app, APP_SCREEN_SETTINGS);
        }
        return;
    }
}
