package org.phash;

import org.junit.Test;
import org.junit.Ignore;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.junit.Assert;

@RunWith(JUnit4.class)
public class TestAudio {
    
    String file1 = "target/test-classes/test.wav";
    String file2 = "target/test-classes/test2.mp3";
    int sr = 8000;
    float nbsecs = 0.0f;

    @Test public void testReadAudio(){
	System.out.println("Start ...");

	AudioMetaData mdataObj = new AudioMetaData();

	float[] samples = AudioData.readAudio(file1, sr, nbsecs, mdataObj);
	Assert.assertNotNull(samples);
	Assert.assertEquals(80877, samples.length);

	Assert.assertNotNull(mdataObj.title2);

	float[] samples2 = AudioData.readAudio(file2, sr, nbsecs, mdataObj);

	Assert.assertNotNull(samples2);
	Assert.assertEquals(45682, samples2.length);
	Assert.assertNotNull(mdataObj.title2);

	return;
    }

}
