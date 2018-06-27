/**

MIT License

Copyright (c) 2018 David G. Starkweather

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

**/

#ifndef _AUDIODATA_H
#define _AUDIODATA_H

#include <stdint.h>

#if defined(BUILD_DLL)
#define AUDIODATA_EXPORT __declspec(dllexport)
#elif defined(BUILD_EXE)
#define AUDIODATA_EXPORT __declspec(dllimport)
#else  
#define AUDIODATA_EXPORT
#endif

typedef struct  ametadata_t {
  char *composer;
  char *title1, *title2, *title3;
  char *tpe1, *tpe2, *tpe3, *tpe4;
  char *date;
  int year;
  char *album;
  char *genre;
  int duration;
  int partofset;
} AudioMetaData;

/* errors for readaudio function */
/* if value of error is < 1000, it is an error from mpg123 (look in mpg123.h for error code) */
/* if value is 0 or >= 1000, then follow this enum */ 
AUDIODATA_EXPORT
enum ph_phashaudio_error {
  PHERR_SUCCESS = 0, 
  PHERR_NULLARG = 1000,
  PHERR_NOBUF =1001,
  PHERR_BADSR = 1002,
  PHERR_NOBUFALLOCD = 1003,
  PHERR_SRCCONTXT = 1004,
  PHERR_SRCPROC = 1005,
  PHERR_SNDFILEOPEN = 1006,
  PHERR_MEMALLOC = 1007,
  PHERR_NOSAMPLES = 1008,
  PHERR_NOFORMAT = 1009,
  PHERR_NOENCODING = 1010,
  PHERR_MP3NEW = 1011,
};
/**
 * init_mdata
 * initialize AudioMetaData struct to all zeros
 **/ 
AUDIODATA_EXPORT
void init_mdata(AudioMetaData *mdata);

/**
 * free_mdata
 * release all memory allocated to fields and zero out.
 **/ 
AUDIODATA_EXPORT
void free_mdata(AudioMetaData *mdata);

/**
 * readaudio function
 * read audio samples into buffer 
 * PARAM filename - char string of filename to read 
 * PARAM sr - int value of desired sample rate of signal
 * PARAM nbsecs - float for number of seconds to take from file - use 0.0 for the whole file.
 * PARAM sigbuf - signal buffer in which to put the extracted signal
 *                (can be NULL for internal allocation of buffer)
 * PARAM buflen - number of elements in signal buffer returned. [in][out] 
 *                [in] the maximum length of the sig buffer (if not set to null)
 *                [out] no. samples in returned buffer
 * PARAM mdata - ptr to AudioMetaData struct to be filled in by function, can be NULL.
 * PARAM error - ptr to int value of error code (0 for success)
 * RETURN ptr to float array
 **/
AUDIODATA_EXPORT
float* readaudio(const char *filename, const int sr,  const float nbsecs, float *sigbuf, int *buflen, 
                                         AudioMetaData *mdata, int *error);

/** free buffer returned by readaudio
 *  @param ptr 
 *  @return void
 **/
AUDIODATA_EXPORT
void freebuffer(void *ptr);

#endif
