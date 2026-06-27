package com.catemup.catchess;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

public final class HttpClient {
    private HttpClient() {
    }

    public static String request(String method, String url, String body, String deviceSecret, int timeoutMs) {
        HttpURLConnection connection = null;

        try {
            int effectiveTimeoutMs = Math.max(1, timeoutMs);
            URL target = new URL(url);
            connection = (HttpURLConnection)target.openConnection();
            connection.setRequestMethod(method);
            connection.setConnectTimeout(effectiveTimeoutMs);
            connection.setReadTimeout(effectiveTimeoutMs);
            connection.setRequestProperty("Accept", "application/json");
            connection.setRequestProperty("Content-Type", "application/json; charset=utf-8");

            if (deviceSecret != null && !deviceSecret.isEmpty()) {
                connection.setRequestProperty("X-Cat-Chess-Device", deviceSecret);
            }

            if (body != null && !body.isEmpty()) {
                byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
                connection.setDoOutput(true);
                connection.setFixedLengthStreamingMode(bytes.length);
                try (OutputStream output = connection.getOutputStream()) {
                    output.write(bytes);
                }
            }

            int status = connection.getResponseCode();
            InputStream input = status >= 400 ? connection.getErrorStream() : connection.getInputStream();
            String responseBody = readAll(input);
            return "{\"status\":" + status + ",\"body\":\"" + escapeJson(responseBody) + "\"}";
        } catch (Exception error) {
            return "{\"status\":0,\"error\":\"" + escapeJson(error.getMessage()) + "\"}";
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    private static String readAll(InputStream input) throws Exception {
        if (input == null) {
            return "";
        }

        StringBuilder builder = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(input, StandardCharsets.UTF_8))) {
            char[] buffer = new char[1024];
            int count;
            while ((count = reader.read(buffer)) >= 0) {
                builder.append(buffer, 0, count);
            }
        }
        return builder.toString();
    }

    private static String escapeJson(String value) {
        if (value == null) {
            return "";
        }

        StringBuilder builder = new StringBuilder();
        for (int index = 0; index < value.length(); ++index) {
            char ch = value.charAt(index);
            if (ch == '\\' || ch == '"') {
                builder.append('\\').append(ch);
            } else if (ch == '\n') {
                builder.append("\\n");
            } else if (ch == '\r') {
                builder.append("\\r");
            } else if (ch == '\t') {
                builder.append("\\t");
            } else {
                builder.append(ch);
            }
        }
        return builder.toString();
    }
}
