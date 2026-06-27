#include "screen_online.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static int screen_online_title_scale(const AppState* app) {
    return app->screenWidth < 420 ? 3 : 4;
}

static int screen_online_group_y(const AppState* app) {
    int top;
    int bottom;
    int available;
    int title_height = screen_online_title_scale(app) * 20;
    int mark_height = app->screenWidth < 420 ? 34 : 48;
    int content_height = title_height
            + UI_SPACE_MD
            + mark_height
            + UI_SPACE_XL
            + UI_BUTTON_HEIGHT_PRIMARY * 2
            + UI_SPACE_MD;

    top = ui_top_safe_padding(app->screenHeight);
    bottom = app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT - UI_SPACE_MD;
    available = bottom - top;
    if (available < content_height) {
        return top;
    }
    return top + (available - content_height) / 2;
}

static UiRect screen_online_button_rect(const AppState* app, int index) {
    int width = ui_content_width(app->screenWidth);
    int start_y = screen_online_group_y(app)
            + screen_online_title_scale(app) * 20
            + UI_SPACE_MD
            + (app->screenWidth < 420 ? 34 : 48)
            + UI_SPACE_XL;

    return ui_centered_rect(
            app->screenWidth,
            start_y + index * (UI_BUTTON_HEIGHT_PRIMARY + UI_SPACE_MD),
            width,
            UI_BUTTON_HEIGHT_PRIMARY
    );
}

static UiRect screen_online_back_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT,
            width,
            UI_BUTTON_HEIGHT_COMPACT
    );
}

void screen_online_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_ONLINE);
    int title_scale = screen_online_title_scale(app);
    int title_y = screen_online_group_y(app);

    ui_draw_centered_label(framebuffer, font, app->screenWidth, title_y, title_scale, UI_COLOR_TEXT, title);
    ui_draw_cat_mark(framebuffer, app->screenWidth / 2, title_y + title_scale * 22, app->screenWidth < 420 ? 2 : 3);
    ui_draw_button_style(
            framebuffer,
            font,
            screen_online_button_rect(app, 0),
            localization_text(app->settings.locale, LOCALIZED_TEXT_CREATE_GAME),
            UI_BUTTON_PRIMARY,
            3
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_online_button_rect(app, 1),
            localization_text(app->settings.locale, LOCALIZED_TEXT_JOIN_GAME),
            UI_BUTTON_PRIMARY,
            3
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_online_back_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_BACK),
            UI_BUTTON_COMPACT,
            2
    );
}

void screen_online_handle_tap(AppState* app, int x, int y) {
    UiRect create_rect;
    UiRect join_rect;
    UiRect back_rect;

    if (app == 0) {
        return;
    }

    create_rect = screen_online_button_rect(app, 0);
    join_rect = screen_online_button_rect(app, 1);
    back_rect = screen_online_back_rect(app);
    if (ui_rect_contains(&create_rect, x, y)) {
        app_navigate(app, APP_SCREEN_CREATE_GAME);
        return;
    }
    if (ui_rect_contains(&join_rect, x, y)) {
        app_navigate(app, APP_SCREEN_JOIN_GAME);
        return;
    }
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
    }
}
