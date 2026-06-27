#include "screen_home.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

#define HOME_BUTTON_COUNT 4
#define HOME_LOGO_ID "main-logo"

static int screen_home_primary_height(const AppState* app) {
    if (app->screenHeight < 640) {
        return 88;
    }
    if (app->screenHeight < 760) {
        return 102;
    }
    return 124;
}

static int screen_home_secondary_height(const AppState* app) {
    if (app->screenHeight < 640) {
        return 76;
    }
    if (app->screenHeight < 760) {
        return 88;
    }
    return 108;
}

static int screen_home_button_gap(const AppState* app) {
    return app->screenHeight < 640 ? UI_SPACE_SM : UI_SPACE_MD;
}

static UiRect screen_home_logo_rect(const AppState* app) {
    UiRect rect;
    int max_width = ui_content_width(app->screenWidth);
    int width = ui_min_int(max_width, app->screenWidth < 420 ? 230 : 280);
    int height = (width * 128) / 256;

    if (height > app->screenHeight / 6) {
        height = app->screenHeight / 6;
        width = (height * 256) / 128;
    }

    rect.width = width;
    rect.height = height;
    rect.x = (app->screenWidth - width) / 2;
    rect.y = ui_centered_group_y(
            app->screenHeight,
            height + UI_SPACE_LG
                    + screen_home_primary_height(app) * 2
                    + screen_home_secondary_height(app) * 2
                    + screen_home_button_gap(app) * 2
                    + (app->screenHeight < 640 ? UI_SPACE_XS : UI_SPACE_SM) * 2
    );
    return rect;
}

static UiRect screen_home_button_rect(const AppState* app, int index) {
    int width = ui_content_width(app->screenWidth);
    UiRect logo = screen_home_logo_rect(app);
    int primary_height = screen_home_primary_height(app);
    int secondary_height = screen_home_secondary_height(app);
    int gap = screen_home_button_gap(app);
    int small_gap = app->screenHeight < 640 ? UI_SPACE_XS : UI_SPACE_SM;
    int total_height = primary_height * 2 + secondary_height * 2 + gap * 2 + small_gap * 2;
    int start_y = logo.y + logo.height + UI_SPACE_LG;
    int max_start_y = app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - total_height;
    int height = index < 2 ? primary_height : secondary_height;
    int y = start_y;

    if (start_y > max_start_y) {
        start_y = ui_max_int(logo.y + logo.height + UI_SPACE_SM, max_start_y);
        y = start_y;
    }

    if (index == 1) {
        y += primary_height + gap;
    } else if (index == 2) {
        y += primary_height * 2 + gap * 2 + small_gap;
    } else if (index == 3) {
        y += primary_height * 2 + secondary_height + gap * 2 + small_gap * 2;
    }

    return ui_centered_rect(app->screenWidth, y, width, height);
}

void screen_home_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const GeneratedSprite* logo = generated_sprite_get_by_id(HOME_LOGO_ID);
    UiRect logo_rect = screen_home_logo_rect(app);
    const LocalizedTextId label_ids[HOME_BUTTON_COUNT] = {
            LOCALIZED_TEXT_LOCAL_GAME,
            LOCALIZED_TEXT_ONLINE,
            LOCALIZED_TEXT_MY_GAMES,
            LOCALIZED_TEXT_SETTINGS
    };

    if (logo != 0) {
        renderer_draw_generated_sprite_fit(framebuffer, logo, logo_rect.x, logo_rect.y, logo_rect.width, logo_rect.height, SPRITE_FIT_CONTAIN);
    } else {
        ui_draw_centered_label(
                framebuffer,
                font,
                app->screenWidth,
                logo_rect.y + logo_rect.height / 3,
                app->screenWidth < 420 ? 3 : 4,
                UI_COLOR_TEXT,
                localization_text(app->settings.locale, LOCALIZED_TEXT_APP_NAME)
        );
    }

    for (int index = 0; index < HOME_BUTTON_COUNT; ++index) {
        ui_draw_button_style(
                framebuffer,
                font,
                screen_home_button_rect(app, index),
                localization_text(app->settings.locale, label_ids[index]),
                index < 2 ? UI_BUTTON_PRIMARY : UI_BUTTON_SECONDARY,
                app->screenHeight >= 760 && index < 2 ? 4 : 3
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
            app_navigate(app, APP_SCREEN_ONLINE);
        } else if (index == 2) {
            app_navigate(app, APP_SCREEN_MY_GAMES);
        } else {
            app_open_settings(app);
        }
        return;
    }
}
