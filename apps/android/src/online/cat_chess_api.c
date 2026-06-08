#include "cat_chess_api.h"

#include <stdio.h>
#include <string.h>

#include "../chess/chess_board.h"
#include "cat_chess_json.h"

#define CAT_CHESS_HTTP_RESPONSE_MAX 8192
#define CAT_CHESS_HTTP_BODY_MAX 6144
#define CAT_CHESS_URL_MAX 256
#define CAT_CHESS_REQUEST_BODY_MAX 256

static JavaVM* api_java_vm;
static jclass api_http_client_class;
static jmethodID api_http_request_method;

static CatChessApiStatus api_status(CatChessApiResult result, int http_status) {
    CatChessApiStatus status;
    status.result = result;
    status.http_status = http_status;
    status.error_code[0] = '\0';
    return status;
}

static void api_copy_error(CatChessApiStatus* status, const char* error_code) {
    if (status == 0 || error_code == 0) {
        return;
    }

    strncpy(status->error_code, error_code, sizeof(status->error_code) - 1u);
    status->error_code[sizeof(status->error_code) - 1u] = '\0';
}

static CatChessApiResult api_result_from_http_status(int http_status) {
    if (http_status >= 200 && http_status < 300) {
        return CAT_CHESS_API_OK;
    }
    if (http_status == 0) {
        return CAT_CHESS_API_NETWORK_ERROR;
    }
    if (http_status == 401) {
        return CAT_CHESS_API_UNAUTHORIZED;
    }
    if (http_status == 404) {
        return CAT_CHESS_API_NOT_FOUND;
    }
    if (http_status == 409) {
        return CAT_CHESS_API_CONFLICT;
    }
    return CAT_CHESS_API_ERROR;
}

void cat_chess_api_bind_android(JavaVM* vm, jobject activity) {
    JNIEnv* env = 0;
    jclass activity_class;
    jmethodID get_class_loader_method;
    jobject class_loader;
    jclass class_loader_class;
    jmethodID load_class_method;
    jstring class_name;
    jclass local_class;

    api_java_vm = vm;
    if (api_java_vm == 0 || activity == 0) {
        return;
    }

    if ((*api_java_vm)->GetEnv(api_java_vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK || env == 0) {
        return;
    }

    activity_class = (*env)->GetObjectClass(env, activity);
    if (activity_class == 0) {
        return;
    }

    get_class_loader_method = (*env)->GetMethodID(env, activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
    if (get_class_loader_method == 0) {
        (*env)->DeleteLocalRef(env, activity_class);
        return;
    }

    class_loader = (*env)->CallObjectMethod(env, activity, get_class_loader_method);
    (*env)->DeleteLocalRef(env, activity_class);
    if (class_loader == 0) {
        return;
    }

    class_loader_class = (*env)->FindClass(env, "java/lang/ClassLoader");
    if (class_loader_class == 0) {
        (*env)->DeleteLocalRef(env, class_loader);
        return;
    }

    load_class_method = (*env)->GetMethodID(env, class_loader_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (load_class_method == 0) {
        (*env)->DeleteLocalRef(env, class_loader_class);
        (*env)->DeleteLocalRef(env, class_loader);
        return;
    }
    class_name = (*env)->NewStringUTF(env, "com.catemup.catchess.HttpClient");
    local_class = (jclass)(*env)->CallObjectMethod(env, class_loader, load_class_method, class_name);
    (*env)->DeleteLocalRef(env, class_name);
    (*env)->DeleteLocalRef(env, class_loader_class);
    (*env)->DeleteLocalRef(env, class_loader);

    if (local_class == 0) {
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
        }
        return;
    }

    api_http_client_class = (jclass)(*env)->NewGlobalRef(env, local_class);
    (*env)->DeleteLocalRef(env, local_class);
    if (api_http_client_class == 0) {
        return;
    }

    api_http_request_method = (*env)->GetStaticMethodID(
            env,
            api_http_client_class,
            "request",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"
    );
}

void cat_chess_api_init(CatChessApiClient* client, const char* base_url) {
    if (client == 0) {
        return;
    }

    client->base_url[0] = '\0';
    client->device_secret[0] = '\0';

    if (base_url != 0 && base_url[0] != '\0') {
        strncpy(client->base_url, base_url, sizeof(client->base_url) - 1u);
        client->base_url[sizeof(client->base_url) - 1u] = '\0';
    }
}

void cat_chess_api_set_device_secret(CatChessApiClient* client, const char* device_secret) {
    if (client == 0) {
        return;
    }

    client->device_secret[0] = '\0';
    if (device_secret != 0 && device_secret[0] != '\0') {
        strncpy(client->device_secret, device_secret, sizeof(client->device_secret) - 1u);
        client->device_secret[sizeof(client->device_secret) - 1u] = '\0';
    }
}

static int api_join_url(const CatChessApiClient* client, const char* path, char* out_url, int out_url_size) {
    int written;

    if (client == 0 || path == 0 || out_url == 0 || out_url_size <= 0 || client->base_url[0] == '\0') {
        return 0;
    }

    written = snprintf(out_url, (size_t)out_url_size, "%s%s", client->base_url, path);
    return written > 0 && written < out_url_size;
}

static JNIEnv* api_get_env(int* did_attach) {
    JNIEnv* env = 0;

    if (did_attach != 0) {
        *did_attach = 0;
    }

    if (api_java_vm == 0) {
        return 0;
    }

    if ((*api_java_vm)->GetEnv(api_java_vm, (void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        return env;
    }

    if ((*api_java_vm)->AttachCurrentThread(api_java_vm, &env, 0) != JNI_OK) {
        return 0;
    }

    if (did_attach != 0) {
        *did_attach = 1;
    }
    return env;
}

static int api_http_request(
        const char* method,
        const char* url,
        const char* body,
        const char* device_secret,
        char* out_response,
        int out_response_size
) {
    JNIEnv* env;
    int did_attach = 0;
    jstring method_string;
    jstring url_string;
    jstring body_string;
    jstring secret_string;
    jstring result_string;
    const char* result_chars;

    if (out_response == 0 || out_response_size <= 0) {
        return 0;
    }
    out_response[0] = '\0';

    env = api_get_env(&did_attach);
    if (env == 0 || api_http_client_class == 0 || api_http_request_method == 0) {
        return 0;
    }

    method_string = (*env)->NewStringUTF(env, method == 0 ? "" : method);
    url_string = (*env)->NewStringUTF(env, url == 0 ? "" : url);
    body_string = body == 0 ? 0 : (*env)->NewStringUTF(env, body);
    secret_string = (*env)->NewStringUTF(env, device_secret == 0 ? "" : device_secret);

    result_string = (jstring)(*env)->CallStaticObjectMethod(
            env,
            api_http_client_class,
            api_http_request_method,
            method_string,
            url_string,
            body_string,
            secret_string
    );

    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        result_string = 0;
    }

    if (result_string != 0) {
        result_chars = (*env)->GetStringUTFChars(env, result_string, 0);
        if (result_chars != 0) {
            strncpy(out_response, result_chars, (size_t)out_response_size - 1u);
            out_response[out_response_size - 1] = '\0';
            (*env)->ReleaseStringUTFChars(env, result_string, result_chars);
        }
    }

    if (method_string != 0) {
        (*env)->DeleteLocalRef(env, method_string);
    }
    if (url_string != 0) {
        (*env)->DeleteLocalRef(env, url_string);
    }
    if (body_string != 0) {
        (*env)->DeleteLocalRef(env, body_string);
    }
    if (secret_string != 0) {
        (*env)->DeleteLocalRef(env, secret_string);
    }
    if (result_string != 0) {
        (*env)->DeleteLocalRef(env, result_string);
    }

    if (did_attach) {
        (*api_java_vm)->DetachCurrentThread(api_java_vm);
    }

    return out_response[0] != '\0';
}

static CatChessApiStatus api_request(
        CatChessApiClient* client,
        const char* method,
        const char* path,
        const char* body,
        int use_auth,
        char* out_body,
        int out_body_size
) {
    char url[CAT_CHESS_URL_MAX];
    char wrapper[CAT_CHESS_HTTP_RESPONSE_MAX];
    char error_code[CAT_CHESS_ERROR_CODE_MAX];
    int http_status = 0;
    CatChessApiStatus status;

    if (out_body != 0 && out_body_size > 0) {
        out_body[0] = '\0';
    }

    if (!api_join_url(client, path, url, sizeof(url))) {
        return api_status(CAT_CHESS_API_ERROR, 0);
    }

    if (!api_http_request(
            method,
            url,
            body,
            use_auth && client != 0 ? client->device_secret : "",
            wrapper,
            sizeof(wrapper)
    )) {
        return api_status(CAT_CHESS_API_NETWORK_ERROR, 0);
    }

    if (!cat_chess_json_parse_http_status(wrapper, &http_status)) {
        return api_status(CAT_CHESS_API_PARSE_ERROR, 0);
    }

    status = api_status(api_result_from_http_status(http_status), http_status);
    cat_chess_json_parse_http_body(wrapper, out_body, out_body_size);
    if (out_body != 0 && cat_chess_json_get_error(out_body, error_code, sizeof(error_code))) {
        api_copy_error(&status, error_code);
    } else if (http_status == 0 && cat_chess_json_get_error(wrapper, error_code, sizeof(error_code))) {
        api_copy_error(&status, error_code);
    }

    return status;
}

int cat_chess_api_is_uci_move(const char* uci) {
    int length = 0;

    if (uci == 0) {
        return 0;
    }

    while (uci[length] != '\0') {
        length += 1;
    }

    if (length != 4 && length != 5) {
        return 0;
    }

    if (uci[0] < 'a' || uci[0] > 'h' || uci[2] < 'a' || uci[2] > 'h') {
        return 0;
    }
    if (uci[1] < '1' || uci[1] > '8' || uci[3] < '1' || uci[3] > '8') {
        return 0;
    }
    if (length == 5 && uci[4] != 'q' && uci[4] != 'r' && uci[4] != 'b' && uci[4] != 'n') {
        return 0;
    }

    return 1;
}

void cat_chess_api_build_uci(int from_square, int to_square, char promotion, char* out_uci, int out_uci_size) {
    int from_file = from_square % CHESS_BOARD_SIZE;
    int from_rank = from_square / CHESS_BOARD_SIZE;
    int to_file = to_square % CHESS_BOARD_SIZE;
    int to_rank = to_square / CHESS_BOARD_SIZE;
    int length = promotion == '\0' ? 4 : 5;

    if (out_uci == 0 || out_uci_size <= 0) {
        return;
    }

    out_uci[0] = '\0';
    if (out_uci_size <= length) {
        return;
    }

    if (from_square < 0
            || from_square >= CHESS_BOARD_SQUARE_COUNT
            || to_square < 0
            || to_square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    if (promotion != '\0' && promotion != 'q' && promotion != 'r' && promotion != 'b' && promotion != 'n') {
        return;
    }

    out_uci[0] = (char)('a' + from_file);
    out_uci[1] = (char)('8' - from_rank);
    out_uci[2] = (char)('a' + to_file);
    out_uci[3] = (char)('8' - to_rank);
    if (promotion != '\0') {
        out_uci[4] = promotion;
    }
    out_uci[length] = '\0';
}

static CatChessApiStatus api_parse_status(CatChessApiStatus status, int parsed) {
    if (status.result == CAT_CHESS_API_OK && !parsed) {
        status.result = CAT_CHESS_API_PARSE_ERROR;
    }
    return status;
}

CatChessApiStatus cat_chess_api_create_or_touch_device(
        CatChessApiClient* client,
        char* out_device_secret,
        int out_device_secret_size
) {
    char body[CAT_CHESS_HTTP_BODY_MAX];
    char request_body[CAT_CHESS_REQUEST_BODY_MAX];
    CatChessApiStatus status;

    if (client != 0 && client->device_secret[0] != '\0') {
        snprintf(request_body, sizeof(request_body), "{\"deviceSecret\":\"%s\"}", client->device_secret);
    } else {
        strncpy(request_body, "{}", sizeof(request_body));
        request_body[sizeof(request_body) - 1] = '\0';
    }

    status = api_request(client, "POST", "/device", request_body, 0, body, sizeof(body));
    return api_parse_status(
            status,
            status.result != CAT_CHESS_API_OK
                    || cat_chess_json_parse_device_secret(body, out_device_secret, out_device_secret_size)
    );
}

CatChessApiStatus cat_chess_api_create_game(
        CatChessApiClient* client,
        CatChessGameDto* out_game
) {
    char body[CAT_CHESS_HTTP_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_game_dto_init(out_game);
    status = api_request(client, "POST", "/games", "{}", 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_game(body, out_game));
}

CatChessApiStatus cat_chess_api_join_game(
        CatChessApiClient* client,
        const char* invite_code,
        CatChessGameDto* out_game
) {
    char body[CAT_CHESS_HTTP_BODY_MAX];
    char request_body[CAT_CHESS_REQUEST_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_game_dto_init(out_game);
    snprintf(request_body, sizeof(request_body), "{\"inviteCode\":\"%s\"}", invite_code == 0 ? "" : invite_code);
    status = api_request(client, "POST", "/games/join", request_body, 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_game(body, out_game));
}

CatChessApiStatus cat_chess_api_list_games(
        CatChessApiClient* client,
        CatChessGameListDto* out_games
) {
    char body[CAT_CHESS_HTTP_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_game_list_dto_init(out_games);
    status = api_request(client, "GET", "/games", 0, 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_games(body, out_games));
}

CatChessApiStatus cat_chess_api_get_game(
        CatChessApiClient* client,
        int game_id,
        CatChessGameDto* out_game
) {
    char path[64];
    char body[CAT_CHESS_HTTP_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_game_dto_init(out_game);
    snprintf(path, sizeof(path), "/games/%d", game_id);
    status = api_request(client, "GET", path, 0, 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_game(body, out_game));
}

CatChessApiStatus cat_chess_api_get_moves(
        CatChessApiClient* client,
        int game_id,
        CatChessMoveListDto* out_moves
) {
    char path[80];
    char body[CAT_CHESS_HTTP_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_move_list_dto_init(out_moves);
    snprintf(path, sizeof(path), "/games/%d/moves", game_id);
    status = api_request(client, "GET", path, 0, 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_moves(body, out_moves));
}

CatChessApiStatus cat_chess_api_make_move(
        CatChessApiClient* client,
        int game_id,
        const char* uci,
        CatChessGameDto* out_game
) {
    char path[80];
    char body[CAT_CHESS_HTTP_BODY_MAX];
    char request_body[CAT_CHESS_REQUEST_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_game_dto_init(out_game);
    snprintf(path, sizeof(path), "/games/%d/move", game_id);
    snprintf(request_body, sizeof(request_body), "{\"uci\":\"%s\"}", uci == 0 ? "" : uci);
    status = api_request(client, "POST", path, request_body, 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_game(body, out_game));
}

CatChessApiStatus cat_chess_api_resign_game(
        CatChessApiClient* client,
        int game_id,
        CatChessGameDto* out_game
) {
    char path[80];
    char body[CAT_CHESS_HTTP_BODY_MAX];
    CatChessApiStatus status;

    cat_chess_game_dto_init(out_game);
    snprintf(path, sizeof(path), "/games/%d/resign", game_id);
    status = api_request(client, "POST", path, "{}", 1, body, sizeof(body));
    return api_parse_status(status, status.result != CAT_CHESS_API_OK || cat_chess_json_parse_game(body, out_game));
}
