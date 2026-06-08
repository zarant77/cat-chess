cmake_minimum_required(VERSION 3.22.1)

project(cat_chess C)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CAT_CHESS_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../..)
set(CAT_CHESS_FONT_OUTPUTS
    ${CAT_CHESS_ROOT}/src/fonts/fonts_packed.c
    ${CAT_CHESS_ROOT}/src/fonts/fonts_packed.h
)
set(CAT_CHESS_SPRITE_OUTPUTS
    ${CAT_CHESS_ROOT}/src/sprites/definitions/sprites_packed.c
    ${CAT_CHESS_ROOT}/src/sprites/definitions/sprites_packed.h
)

file(GLOB_RECURSE CAT_CHESS_FONT_ASSETS CONFIGURE_DEPENDS
    ${CAT_CHESS_ROOT}/src/assets/fonts/*.font.json
)
file(GLOB_RECURSE CAT_CHESS_SPRITE_ASSETS CONFIGURE_DEPENDS
    ${CAT_CHESS_ROOT}/src/assets/sprites/*.json
)

add_custom_command(
    OUTPUT ${CAT_CHESS_FONT_OUTPUTS}
    COMMAND ${Python3_EXECUTABLE} ${CAT_CHESS_ROOT}/tools/pack_fonts.py
    DEPENDS
        ${CAT_CHESS_ROOT}/tools/pack_fonts.py
        ${CAT_CHESS_FONT_ASSETS}
    WORKING_DIRECTORY ${CAT_CHESS_ROOT}
    COMMENT "Packing vector fonts"
)

add_custom_target(pack_fonts DEPENDS ${CAT_CHESS_FONT_OUTPUTS})

add_custom_command(
    OUTPUT ${CAT_CHESS_SPRITE_OUTPUTS}
    COMMAND ${Python3_EXECUTABLE} ${CAT_CHESS_ROOT}/tools/pack_sprites.py
    DEPENDS
        ${CAT_CHESS_ROOT}/tools/pack_sprites.py
        ${CAT_CHESS_SPRITE_ASSETS}
    WORKING_DIRECTORY ${CAT_CHESS_ROOT}
    COMMENT "Packing vector sprites"
)

add_custom_target(pack_sprites DEPENDS ${CAT_CHESS_SPRITE_OUTPUTS})

add_library(cat_chess SHARED
    ../../../../src/app/app.c
    ../../../../src/audio/chess_sound.c
    ../../../../src/chess/chess_ai.c
    ../../../../src/chess/chess_board.c
    ../../../../src/chess/chess_game.c
    ../../../../src/chess/chess_move.c
    ../../../../src/chess/chess_rules.c
    ../../../../src/fonts/font_renderer.c
    ../../../../src/fonts/fonts_packed.c
    ../../../../src/input/input.c
    ../../../../src/localization/localization.c
    ../../../../src/main.c
    ../../../../src/online/cat_chess_api.c
    ../../../../src/online/cat_chess_json.c
    ../../../../src/online/cat_chess_models.c
    ../../../../src/online/online_game.c
    ../../../../src/platform/platform_android.c
    ../../../../src/renderer/renderer.c
    ../../../../src/settings/game_settings.c
    ../../../../src/storage/local_storage.c
    ../../../../src/sprites/definitions/sprites_packed.c
    ../../../../src/sprites/generated_sprite.c
    ../../../../src/ui/chess_board_view.c
    ../../../../src/ui/chess_piece_view.c
    ../../../../src/ui/screens/screen_create_game.c
    ../../../../src/ui/screens/screen_game.c
    ../../../../src/ui/screens/screen_home.c
    ../../../../src/ui/screens/screen_join_game.c
    ../../../../src/ui/screens/screen_local_game.c
    ../../../../src/ui/screens/screen_my_games.c
    ../../../../src/ui/screens/screen_settings.c
    ../../../../src/ui/ui_controls.c
)

add_dependencies(cat_chess pack_fonts pack_sprites)

target_include_directories(cat_chess PRIVATE
    ../../../../src
)

target_link_libraries(cat_chess android log)
