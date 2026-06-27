#!/usr/bin/env bash

set -euo pipefail

APP_ID="com.catemup.catchess"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="$ROOT_DIR/apps/android"

SOURCE_APK="$ANDROID_DIR/app/build/outputs/apk/release/app-release-unsigned.apk"
BUILD_DIR="$ROOT_DIR/build"
OUTPUT_APK="$BUILD_DIR/cat-chess.apk"

SDK="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
BUILD_TOOLS_DIR="$(ls -d "$SDK"/build-tools/* | sort -V | tail -n 1)"
APKSIGNER="$BUILD_TOOLS_DIR/apksigner"
KEYSTORE="$HOME/.android/debug.keystore"

ensure_tools() {
    if [ ! -d "$ANDROID_DIR" ]; then
        echo "ERROR: Android project not found: $ANDROID_DIR"
        exit 1
    fi
    
    if [ ! -x "$ANDROID_DIR/gradlew" ]; then
        echo "ERROR: gradlew not found or not executable: $ANDROID_DIR/gradlew"
        exit 1
    fi
    
    if [ ! -x "$APKSIGNER" ]; then
        echo "ERROR: apksigner not found: $APKSIGNER"
        exit 1
    fi
}

ensure_keystore() {
    if [ -f "$KEYSTORE" ]; then
        return
    fi
    
    echo "Creating debug keystore..."
    mkdir -p "$HOME/.android"
    
    keytool -genkeypair \
    -v \
    -keystore "$KEYSTORE" \
    -storepass android \
    -alias androiddebugkey \
    -keypass android \
    -keyalg RSA \
    -keysize 2048 \
    -validity 10000 \
    -dname "CN=Android Debug,O=Android,C=US"
}

show_logo() {
  cat <<'EOF'
   ____      _      ____ _
  / ___|__ _| |_   / ___| |__   ___  ___ ___
 | |   / _` | __| | |   | '_ \ / _ \/ __/ __|
 | |__| (_| | |_  | |___| | | |  __/\__ \__ \
  \____\__,_|\__|  \____|_| |_|\___||___/___/

        /\_/\        ♟ ♞ ♜ ♛ ♚
       ( o.o )       No accounts.
        > ^ <        Just chess.
EOF
}

pack_assets() {
    echo
    echo "Packing assets JSON files..."

    (
        cd "$ANDROID_DIR"
        python3 tools/pack_music.py
        python3 tools/pack_animations.py
        python3 tools/pack_fonts.py
        python3 tools/pack_sprites.py
    )
}

sign_apk() {
    if [ ! -f "$SOURCE_APK" ]; then
        echo "ERROR: Source APK not found: $SOURCE_APK"
        exit 1
    fi
    
    "$APKSIGNER" sign \
    --ks "$KEYSTORE" \
    --ks-key-alias androiddebugkey \
    --ks-pass pass:android \
    --key-pass pass:android \
    "$SOURCE_APK"
}

verify_apk() {
    "$APKSIGNER" verify "$SOURCE_APK"
}

build_apk() {
    ensure_tools
    ensure_keystore
    
    pack_assets
    
    echo
    echo "SDK: $SDK"
    echo "Build Tools: $BUILD_TOOLS_DIR"
    echo "APK Signer: $APKSIGNER"
    echo "Android Dir: $ANDROID_DIR"
    echo
    
    rm -rf "$ANDROID_DIR/app/build"
    rm -rf "$ANDROID_DIR/app/.cxx"
    
    (
        cd "$ANDROID_DIR"
        ./gradlew assembleRelease
    )
    
    sign_apk
    verify_apk
    
    mkdir -p "$BUILD_DIR"
    cp "$SOURCE_APK" "$OUTPUT_APK"
    
    show_apk_size
}

install_apk() {
    if [ ! -f "$OUTPUT_APK" ]; then
        echo "ERROR: APK not found. Build first."
        exit 1
    fi
    
    adb install -r -d "$OUTPUT_APK"
}

launch_app() {
    adb shell monkey -p "$APP_ID" 1
}

show_logs() {
    adb logcat -c
    adb logcat | grep CatChess
}

clean_project() {
    rm -rf "$ROOT_DIR/build"
    rm -rf "$ROOT_DIR/app"
    rm -rf "$ROOT_DIR/.gradle"
    
    rm -rf "$ANDROID_DIR/.gradle"
    rm -rf "$ANDROID_DIR/build"
    rm -rf "$ANDROID_DIR/app/build"
    rm -rf "$ANDROID_DIR/app/.cxx"
}

show_apk_size() {
    if [ ! -f "$OUTPUT_APK" ]; then
        echo "ERROR: APK not found."
        return
    fi
    
    echo
    echo "APK:"
    ls -lh "$OUTPUT_APK"
    
    echo
    echo "APK contents:"
    unzip -l "$OUTPUT_APK" | tail -n 20
}

show_devices() {
    adb devices
}

clear
show_logo

echo
echo "1) Build + Install + Launch"
echo "2) Build"
echo "3) Pack Assets JSON -> C"
echo "4) Clean"
echo "5) Logs"
echo "6) APK size"
echo "7) Devices"
echo "0) Exit"
echo

read -rp "> " choice

case "$choice" in
    1) build_apk; install_apk; launch_app ;;
    2) build_apk ;;
    3) pack_assets ;;
    4) clean_project ;;
    5) show_logs ;;
    6) show_apk_size ;;
    7) show_devices ;;
    0) exit 0 ;;
    *) echo "Invalid option"; exit 1 ;;
esac
