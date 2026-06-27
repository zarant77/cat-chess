package com.catemup.catchess;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;

public final class CatChessAudio {
    private static final int SAMPLE_RATE = 22050;
    private static AudioTrack musicTrack;
    private static String currentMusicId;

    private CatChessAudio() {
    }

    public static void play(String id, int volume) {
        SoundSpec spec = specFor(id);
        if (spec == null || volume <= 0) {
            return;
        }

        final float gain = Math.max(0.0f, Math.min(1.0f, volume / 100.0f)) * spec.gain;
        final short[] samples = render(spec, gain);
        final byte[] pcm = new byte[samples.length * 2];

        for (int index = 0; index < samples.length; index += 1) {
            pcm[index * 2] = (byte)(samples[index] & 0xff);
            pcm[index * 2 + 1] = (byte)((samples[index] >> 8) & 0xff);
        }

        Thread thread = new Thread(() -> playPcm(pcm), "CatChessAudio");
        thread.setDaemon(true);
        thread.start();
    }

    public static synchronized void playMusicPcm(String id, short[] samples, int sampleRate, int loopStart, int loopEnd, int volume) {
        if (id == null || id.isEmpty() || samples == null || samples.length <= 0 || sampleRate <= 0 || volume <= 0) {
            return;
        }
        if (musicTrack != null && id.equals(currentMusicId)) {
            return;
        }

        stopMusic();

        final int safeLoopStart = Math.max(0, Math.min(loopStart, samples.length - 1));
        final int safeLoopEnd = Math.max(safeLoopStart + 1, Math.min(loopEnd, samples.length));
        final float gain = Math.max(0.0f, Math.min(1.0f, volume / 100.0f)) * 0.18f;
        final byte[] pcm = samplesToPcm(samples, gain);

        try {
            musicTrack = new AudioTrack.Builder()
                    .setAudioAttributes(new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_GAME)
                            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                            .build())
                    .setAudioFormat(new AudioFormat.Builder()
                            .setSampleRate(sampleRate)
                            .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                            .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                            .build())
                    .setTransferMode(AudioTrack.MODE_STATIC)
                    .setBufferSizeInBytes(pcm.length)
                    .build();
            musicTrack.write(pcm, 0, pcm.length);
            musicTrack.setLoopPoints(safeLoopStart, safeLoopEnd, -1);
            currentMusicId = id;
            musicTrack.play();
        } catch (RuntimeException ignored) {
            stopMusic();
        }
    }

    public static synchronized void stopMusic() {
        if (musicTrack == null) {
            currentMusicId = null;
            return;
        }
        try {
            musicTrack.pause();
            musicTrack.flush();
        } catch (RuntimeException ignored) {
        }
        musicTrack.release();
        musicTrack = null;
        currentMusicId = null;
    }

    private static void playPcm(byte[] pcm) {
        AudioTrack track = null;
        try {
            track = new AudioTrack.Builder()
                    .setAudioAttributes(new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_GAME)
                            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                            .build())
                    .setAudioFormat(new AudioFormat.Builder()
                            .setSampleRate(SAMPLE_RATE)
                            .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                            .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                            .build())
                    .setTransferMode(AudioTrack.MODE_STATIC)
                    .setBufferSizeInBytes(pcm.length)
                    .build();
            track.write(pcm, 0, pcm.length);
            track.play();
        } catch (RuntimeException ignored) {
        } finally {
            if (track != null) {
                try {
                    Thread.sleep(Math.max(20L, (pcm.length / 2L) * 1000L / SAMPLE_RATE));
                } catch (InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                }
                track.release();
            }
        }
    }

    private static byte[] samplesToPcm(short[] samples, float gain) {
        final byte[] pcm = new byte[samples.length * 2];

        for (int index = 0; index < samples.length; index += 1) {
            int sample = Math.round(samples[index] * gain);
            sample = Math.max(Short.MIN_VALUE, Math.min(Short.MAX_VALUE, sample));
            pcm[index * 2] = (byte)(sample & 0xff);
            pcm[index * 2 + 1] = (byte)((sample >> 8) & 0xff);
        }

        return pcm;
    }

    private static short[] render(SoundSpec spec, float gain) {
        int sampleCount = Math.max(1, (SAMPLE_RATE * spec.durationMs) / 1000);
        short[] samples = new short[sampleCount];
        double phase = 0.0;

        for (int index = 0; index < sampleCount; index += 1) {
            double t = sampleCount <= 1 ? 1.0 : (double)index / (double)(sampleCount - 1);
            double frequency = spec.startHz + (spec.endHz - spec.startHz) * t;
            double envelope = envelope(t);
            double body = Math.sin(phase);
            double click = Math.sin(phase * 2.01) * 0.18;
            double sample = (body + click) * envelope * gain;

            phase += (Math.PI * 2.0 * frequency) / SAMPLE_RATE;
            samples[index] = (short)Math.max(Short.MIN_VALUE, Math.min(Short.MAX_VALUE, sample * 32767.0));
        }

        return samples;
    }

    private static double envelope(double t) {
        if (t < 0.08) {
            return t / 0.08;
        }
        return Math.pow(1.0 - t, 1.6);
    }

    private static SoundSpec specFor(String id) {
        if ("sfx_move".equals(id)) {
            return new SoundSpec(56, 560.0, 420.0, 0.28f);
        }
        if ("sfx_capture".equals(id)) {
            return new SoundSpec(92, 360.0, 220.0, 0.34f);
        }
        if ("sfx_check".equals(id)) {
            return new SoundSpec(120, 780.0, 980.0, 0.28f);
        }
        if ("sfx_checkmate".equals(id)) {
            return new SoundSpec(240, 520.0, 880.0, 0.30f);
        }
        if ("sfx_illegal".equals(id)) {
            return new SoundSpec(96, 240.0, 160.0, 0.24f);
        }
        if ("sfx_menu_select".equals(id)) {
            return new SoundSpec(44, 680.0, 580.0, 0.20f);
        }
        if ("sfx_menu_back".equals(id)) {
            return new SoundSpec(48, 420.0, 320.0, 0.18f);
        }
        if ("sfx_game_created".equals(id)) {
            return new SoundSpec(160, 520.0, 760.0, 0.27f);
        }
        if ("sfx_game_joined".equals(id)) {
            return new SoundSpec(150, 600.0, 820.0, 0.27f);
        }
        return null;
    }

    private static final class SoundSpec {
        final int durationMs;
        final double startHz;
        final double endHz;
        final float gain;

        SoundSpec(int durationMs, double startHz, double endHz, float gain) {
            this.durationMs = durationMs;
            this.startHz = startHz;
            this.endHz = endHz;
            this.gain = gain;
        }
    }
}
