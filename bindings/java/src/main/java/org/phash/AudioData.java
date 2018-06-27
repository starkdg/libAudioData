package org.phash;

public class AudioData {

    public native static float[] readAudio(String filename, int sr, float nbsecs, AudioMetaData mdataObj);

    static {
	System.loadLibrary("AudioData-jni");
    }

}
