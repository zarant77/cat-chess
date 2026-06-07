#include "localization.h"

typedef struct {
    const char* english;
    const char* ukrainian;
} LocalizedText;

static const LocalizedText LOCALIZED_TEXTS[] = {
    [LOCALIZED_TEXT_APP_NAME] = {"CAT CHESS", "КОТОШАХИ"},
    [LOCALIZED_TEXT_NEW_GAME] = {"NEW GAME", "НОВА ГРА"},
    [LOCALIZED_TEXT_RESUME] = {"RESUME", "ПРОДОВЖИТИ"},
    [LOCALIZED_TEXT_RESTART] = {"RESTART", "ПОЧАТИ ЗАНОВО"},
    [LOCALIZED_TEXT_SETTINGS] = {"SETTINGS", "НАЛАШТУВАННЯ"},
    [LOCALIZED_TEXT_BACK] = {"BACK", "НАЗАД"},
    [LOCALIZED_TEXT_EXIT] = {"EXIT", "ВИХІД"},
    [LOCALIZED_TEXT_LANGUAGE] = {"LANGUAGE", "МОВА"},
    [LOCALIZED_TEXT_LANGUAGE_ENGLISH] = {"EN", "АНГЛ"},
    [LOCALIZED_TEXT_LANGUAGE_UKRAINIAN] = {"UK", "УКР"},
    [LOCALIZED_TEXT_SFX] = {"SFX", "ЗВУКИ"},
    [LOCALIZED_TEXT_GAME_OVER] = {"GAME OVER", "КІНЕЦЬ ГРИ"},
    [LOCALIZED_TEXT_GAME_START] = {"GAME START", "ПОЧАТОК ГРИ"},
    [LOCALIZED_TEXT_GAME_END] = {"GAME END", "КІНЕЦЬ ПАРТІЇ"},
    [LOCALIZED_TEXT_WHITE] = {"WHITE", "БІЛІ"},
    [LOCALIZED_TEXT_BLACK] = {"BLACK", "ЧОРНІ"},
    [LOCALIZED_TEXT_WHITE_TO_MOVE] = {"WHITE TO MOVE", "ХІД БІЛИХ"},
    [LOCALIZED_TEXT_BLACK_TO_MOVE] = {"BLACK TO MOVE", "ХІД ЧОРНИХ"},
    [LOCALIZED_TEXT_SELECT_PIECE] = {"SELECT PIECE", "ОБЕРІТЬ ФІГУРУ"},
    [LOCALIZED_TEXT_SELECT_TARGET] = {"SELECT TARGET", "ОБЕРІТЬ КЛІТИНКУ"},
    [LOCALIZED_TEXT_MOVE] = {"MOVE", "ХІД"},
    [LOCALIZED_TEXT_CAPTURE] = {"CAPTURE", "ВЗЯТТЯ"},
    [LOCALIZED_TEXT_ILLEGAL_MOVE] = {"ILLEGAL MOVE", "НЕДОПУСТИМИЙ ХІД"},
    [LOCALIZED_TEXT_CHECK] = {"CHECK", "ШАХ"},
    [LOCALIZED_TEXT_CHECKMATE] = {"CHECKMATE", "МАТ"},
    [LOCALIZED_TEXT_STALEMATE] = {"STALEMATE", "ПАТ"},
    [LOCALIZED_TEXT_DRAW] = {"DRAW", "НІЧИЯ"},
    [LOCALIZED_TEXT_CASTLE] = {"CASTLE", "РОКІРУВАННЯ"},
    [LOCALIZED_TEXT_PROMOTION] = {"PROMOTION", "ПЕРЕТВОРЕННЯ"},
    [LOCALIZED_TEXT_PROMOTE_TO] = {"PROMOTE TO", "ПЕРЕТВОРИТИ НА"},
    [LOCALIZED_TEXT_UNDO] = {"UNDO", "СКАСУВАТИ"},
    [LOCALIZED_TEXT_REDO] = {"REDO", "ПОВТОРИТИ"},
    [LOCALIZED_TEXT_PAWN] = {"PAWN", "ПІШАК"},
    [LOCALIZED_TEXT_KNIGHT] = {"KNIGHT", "КІНЬ"},
    [LOCALIZED_TEXT_BISHOP] = {"BISHOP", "СЛОН"},
    [LOCALIZED_TEXT_ROOK] = {"ROOK", "ТУРА"},
    [LOCALIZED_TEXT_QUEEN] = {"QUEEN", "ФЕРЗЬ"},
    [LOCALIZED_TEXT_KING] = {"KING", "КОРОЛЬ"},
    [LOCALIZED_TEXT_LOCAL_GAME] = {"LOCAL GAME", "ЛОКАЛЬНА ГРА"},
    [LOCALIZED_TEXT_VS_COMPUTER] = {"VS COMPUTER", "ПРОТИ КОМП'ЮТЕРА"},
    [LOCALIZED_TEXT_THINKING] = {"THINKING", "ДУМАЮ"},
    [LOCALIZED_TEXT_YOU_WIN] = {"YOU WIN", "ВИ ПЕРЕМОГЛИ"},
    [LOCALIZED_TEXT_YOU_LOSE] = {"YOU LOSE", "ВИ ПРОГРАЛИ"},
};

const char* localization_text(GameLocale locale, LocalizedTextId text_id) {
    if (text_id < 0 || (int)text_id >= (int)(sizeof(LOCALIZED_TEXTS) / sizeof(LOCALIZED_TEXTS[0]))) {
        return "";
    }

    if (game_settings_normalize_locale(locale) == GAME_LOCALE_UKRAINIAN) {
        return LOCALIZED_TEXTS[text_id].ukrainian;
    }

    return LOCALIZED_TEXTS[text_id].english;
}
