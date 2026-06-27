package com.catemup.catchess;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.text.method.TextKeyListener;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputConnectionWrapper;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;

public final class SoftKeyboard {
    private static EditText editor;
    private static boolean clearingText;
    private static final StringBuilder pendingText = new StringBuilder();
    private static int pendingBackspaces;

    private SoftKeyboard() {
    }

    private static void sendTextInput(char value) {
        synchronized (SoftKeyboard.class) {
            pendingText.append(value);
        }
    }

    private static void sendBackspace() {
        synchronized (SoftKeyboard.class) {
            pendingBackspaces += 1;
        }
    }

    public static synchronized String takePendingText() {
        String text = pendingText.toString();
        pendingText.setLength(0);
        return text;
    }

    public static synchronized int takePendingBackspaces() {
        int count = pendingBackspaces;
        pendingBackspaces = 0;
        return count;
    }

    public static void show(Activity activity) {
        if (activity == null) {
            return;
        }

        activity.runOnUiThread(() -> {
            EditText view = ensureEditor(activity);
            view.requestFocus();

            InputMethodManager inputMethodManager =
                    (InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
            if (inputMethodManager != null) {
                if (!inputMethodManager.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT)) {
                    inputMethodManager.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
                }
            }
        });
    }

    private static EditText ensureEditor(Activity activity) {
        if (editor != null && editor.getContext() == activity) {
            return editor;
        }

        NativeInputEditText nextEditor = new NativeInputEditText(activity);
        nextEditor.setSingleLine(true);
        nextEditor.setInputType(
                android.text.InputType.TYPE_CLASS_TEXT
                        | android.text.InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS
                        | android.text.InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
                        | android.text.InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
        nextEditor.setImeOptions(EditorInfo.IME_FLAG_NO_EXTRACT_UI);
        nextEditor.setKeyListener(TextKeyListener.getInstance(false, TextKeyListener.Capitalize.CHARACTERS));
        nextEditor.setFilters(new InputFilter[] {new InputFilter.AllCaps()});
        nextEditor.setTextColor(Color.TRANSPARENT);
        nextEditor.setBackgroundColor(Color.TRANSPARENT);
        nextEditor.setAlpha(0.01f);
        nextEditor.setFocusable(true);
        nextEditor.setFocusableInTouchMode(true);
        nextEditor.setOnKeyListener((view, keyCode, event) -> {
            if (keyCode == KeyEvent.KEYCODE_DEL && event.getAction() == KeyEvent.ACTION_DOWN) {
                sendBackspace();
                return true;
            }
            return false;
        });
        nextEditor.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence text, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence text, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable text) {
                if (clearingText || text.length() == 0) {
                    return;
                }

                for (int index = 0; index < text.length(); ++index) {
                    char value = Character.toUpperCase(text.charAt(index));
                    if ((value >= 'A' && value <= 'Z') || (value >= '0' && value <= '9')) {
                        sendTextInput(value);
                    }
                }

                clearingText = true;
                text.clear();
                clearingText = false;
            }
        });

        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(1, 1);
        params.leftMargin = 0;
        params.topMargin = 0;
        activity.addContentView(nextEditor, params);
        editor = nextEditor;
        return editor;
    }

    private static final class NativeInputEditText extends EditText {
        NativeInputEditText(Context context) {
            super(context);
        }

        @Override
        public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
            InputConnection connection = super.onCreateInputConnection(outAttrs);
            if (connection == null) {
                return null;
            }

            return new InputConnectionWrapper(connection, true) {
                @Override
                public boolean deleteSurroundingText(int beforeLength, int afterLength) {
                    if (beforeLength > 0) {
                        sendBackspace();
                    }
                    return super.deleteSurroundingText(beforeLength, afterLength);
                }

                @Override
                public boolean sendKeyEvent(KeyEvent event) {
                    if (event.getKeyCode() == KeyEvent.KEYCODE_DEL && event.getAction() == KeyEvent.ACTION_DOWN) {
                        sendBackspace();
                        return true;
                    }
                    return super.sendKeyEvent(event);
                }
            };
        }
    }
}
