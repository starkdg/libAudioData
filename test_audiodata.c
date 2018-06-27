/*
    Audio Scout - audio content indexing software
    Copyright (C) 2010  D. Grant Starkweather
    
    Audio Scout is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    David Starkweather - dstarkweather@phash.org
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "audiodata.h"

void print_metadata(AudioMetaData *mdata){
    printf("composer: %s\n", mdata->composer);
    printf("title1: %s\n", mdata->title1);
    printf("title2: %s\n", mdata->title2);
    printf("title3: %s\n", mdata->title3);
    printf("tpe1:   %s\n", mdata->tpe1);
    printf("tpe2:   %s\n", mdata->tpe2);
    printf("tpe3:   %s\n", mdata->tpe3);
    printf("tpe4:   %s\n", mdata->tpe4);
    printf("date    %s\n", mdata->date);
    printf("year:   %d\n", mdata->year);
    printf("album:  %s\n", mdata->album);
    printf("genre:  %s\n", mdata->genre);
    printf("dur:    %d\n", mdata->duration);
    printf("pos:    %d\n", mdata->partofset);
}

int main(int argc, char **argv){

    const char *file1 = argv[1];
    const char *file2 = argv[2];
    const int sr = 8000;

    int error;
    int buflen = 0;
    AudioMetaData mdata;
    float *buf = readaudio(file1, sr, 0, NULL, &buflen, &mdata, &error);
    assert(buf != NULL);
    assert(buflen == 80877);
    assert(error == 0);

    int buflen2 = 0;
    AudioMetaData mdata2;
    float *buf2 = readaudio(file2, sr, 0, NULL, &buflen2, &mdata2, &error);
    assert(buf2 != NULL);
    assert(buflen2 == 45682);
    assert(error == 0);
    
    free_mdata(&mdata);
    free_mdata(&mdata2);
    freebuffer(buf);
    freebuffer(buf2);

    return 0;
}
