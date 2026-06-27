#pragma once

/*
 * Local Android emulator builds should use http://10.0.2.2:5400 to reach the
 * host machine's Cat Chess server.
 *
 * A real Android device on the same Wi-Fi should use the computer's LAN IP,
 * for example http://192.168.0.52:5400.
 *
 * Production deployments should use an HTTPS URL.
 */
#define CAT_CHESS_SERVER_URL "http://192.168.0.52:5400"
#define CAT_CHESS_HTTP_TIMEOUT_MS 5000
#define CAT_CHESS_POLL_INTERVAL_MS 1500
