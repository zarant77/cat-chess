#include "screen_home.h"

#include "../../fonts/font_renderer.h"
#include "../ui_controls.h"

#define HOME_BUTTON_COUNT 5

static UiRect screen_home_button_rect(const AppState* app, int index) {
    int width = app->screenWidth < 520 ? app->screenWidth - 48 : 520;
    int height = 58;
    int gap = 14;
    int total_height = HOME_BUTTON_COUNT * height + (HOME_BUTTON_COUNT - 1) * gap;
    int start_y = (app->screenHeight - total_height) / 2 + 34;
    UiRect rect;

    if (width < 220) {
        width = 220;
    }

    rect.x = (app->screenWidth - width) / 2;
    rect.y = start_y + index * (height + gap);
    rect.width = width;
    rect.height = height;
    return rect;
}

void screen_home_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* labels[HOME_BUTTON_COUNT] = {
            "Local Game",
            "Create Online Game",
            "Join Online Game",
            "My Games",
            "Settings"
    };
    int title_scale = app->screenWidth < 420 ? 3 : 4;
    int title_width = font_measure_text(font, title_scale, "Cat Chess");
    int title_y = app->screenHeight / 2 - 220;

    if (title_y < 36) {
        title_y = 36;
    }

    ui_draw_label(
            framebuffer,
            font,
            (app->screenWidth - title_width) / 2,
            title_y,
            title_scale,
            0x27312bffu,
            "Cat Chess"
    );

    for (int index = 0; index < HOME_BUTTON_COUNT; ++index) {
        ui_draw_button(framebuffer, font, screen_home_button_rect(app, index), labels[index]);
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
