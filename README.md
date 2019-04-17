### libAudioData v0.0.1

	Unified wrapper interface to read audio data from files at a
	chosen sample rate.  All stereo channels are merged into one channel.
	All meta data is extracted as well. 

	List of file formats supported: .wav, .mp3, .mp2, .amr, .aifc, .ogg, .flac

## Install

You can install the library like so:

```
cmake .
make all
make install
```

The java bindings' native jni library:

```
cd bindings/java
cmake .
make all
make install
```

Install the java jar package in the local maven .m2 repository:

```
mvn install
```

## Dependencies
   (With the development header files)

	- libsndfile      v1.0 .21 (www.meganerd.com/libsndfile)
    - libsamplerate   v0.1 .7  (www.meganerd.com/src)
    - libmpg123       v0.25.1  (www.mpg123.de)
    - opencore-amrnb  v0.0 .3  (sourceforge.net/projects/opencore-amr)
    - opencore-amrwb  v0.0 .3 
