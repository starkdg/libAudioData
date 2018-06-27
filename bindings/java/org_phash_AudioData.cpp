#include <stdint.h>
#include <jni.h>
#include <stdlib.h>
#include "org_phash_AudioData.h"

extern "C" {
#include "../../audiodata.h"
}

JNIEXPORT jfloatArray JNICALL Java_org_phash_AudioData_readAudio
                     (JNIEnv *env, jclass cl, jstring name, jint sr, jfloat nbsecs, jobject mdataObj){

    jboolean iscopy;
    const char *filename = env->GetStringUTFChars(name, &iscopy);

    int error;
    int buflen = 0;
    AudioMetaData mdata;
    init_mdata(&mdata);
    float *buf = readaudio(filename, sr, nbsecs, NULL, &buflen, &mdata, &error);
    if (buf == NULL){
	free_mdata(&mdata);
	env->ReleaseStringUTFChars(name, filename);
	return NULL;
    }
    jfloatArray buf2 = env->NewFloatArray((jint)buflen);
    env->SetFloatArrayRegion(buf2, 0, buflen, (jfloat*)buf);

    // get fieldIds
    jclass mdataclass      = env->GetObjectClass(mdataObj);

    jfieldID composerfield  = env->GetFieldID(mdataclass, "composer" , "Ljava/lang/String;" );
    jfieldID title1field    = env->GetFieldID(mdataclass, "title1"   , "Ljava/lang/String;" );
    jfieldID title2field    = env->GetFieldID(mdataclass, "title2"   , "Ljava/lang/String;" );
    jfieldID title3field    = env->GetFieldID(mdataclass, "title3"   , "Ljava/lang/String;" );
    jfieldID tpe1field      = env->GetFieldID(mdataclass, "tpe1"     , "Ljava/lang/String;" );
    jfieldID tpe2field      = env->GetFieldID(mdataclass, "tpe2"     , "Ljava/lang/String;" );
    jfieldID tpe3field      = env->GetFieldID(mdataclass, "tpe3"     , "Ljava/lang/String;" );
    jfieldID tpe4field      = env->GetFieldID(mdataclass, "tpe4"     , "Ljava/lang/String;" );
    jfieldID datefield      = env->GetFieldID(mdataclass, "date"     , "Ljava/lang/String;" );
    jfieldID albumfield     = env->GetFieldID(mdataclass, "album"    , "Ljava/lang/String;" );
    jfieldID genrefield     = env->GetFieldID(mdataclass, "genre"    , "Ljava/lang/String;" );
    jfieldID yearfield      = env->GetFieldID(mdataclass, "year"     , "I");
    jfieldID durationfield  = env->GetFieldID(mdataclass, "duration" , "I");
    jfieldID partofsetfield = env->GetFieldID(mdataclass, "partofset", "I");
    
    // set AudioMetaData fields 
    if (composerfield != 0) env->SetObjectField(mdataObj, composerfield,  env->NewStringUTF(mdata.composer));
    if (title1field != 0)   env->SetObjectField(mdataObj, title1field,    env->NewStringUTF(mdata.title1));
    if (title2field != 0)   env->SetObjectField(mdataObj, title2field,    env->NewStringUTF(mdata.title2));
    if (title3field != 0)   env->SetObjectField(mdataObj, title3field,    env->NewStringUTF(mdata.title3));
    if (tpe1field   != 0)   env->SetObjectField(mdataObj, tpe1field,      env->NewStringUTF(mdata.tpe1));
    if (tpe2field   != 0)   env->SetObjectField(mdataObj, tpe2field,      env->NewStringUTF(mdata.tpe2));
    if (tpe3field   != 0)   env->SetObjectField(mdataObj, tpe3field,      env->NewStringUTF(mdata.tpe3));
    if (tpe4field   != 0)   env->SetObjectField(mdataObj, tpe4field,      env->NewStringUTF(mdata.tpe4));
    if (datefield   != 0)   env->SetObjectField(mdataObj, datefield,      env->NewStringUTF(mdata.date));
    if (albumfield  != 0)   env->SetObjectField(mdataObj, albumfield,     env->NewStringUTF(mdata.album));
    if (genrefield  != 0)   env->SetObjectField(mdataObj, genrefield,     env->NewStringUTF(mdata.genre));
    if (yearfield   != 0)   env->SetIntField(mdataObj   , yearfield,      mdata.year);
    if (durationfield != 0) env->SetIntField(mdataObj   , durationfield,  mdata.duration);
    if (partofsetfield != 0)env->SetIntField(mdataObj   , partofsetfield, mdata.partofset);

    free(buf);
    free_mdata(&mdata);
    env->ReleaseStringUTFChars(name, filename);

    return buf2;
}
}
