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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "audiodata.h"
#include "sndfile.h"
#include "samplerate.h"
#include "AudioDataConfig.h"

#ifdef HAVE_MPG123
#include "mpg123.h"
#endif

#ifdef HAVE_AMR
#include "opencore-amrnb/interf_dec.h"
#endif

void init_mdata(AudioMetaData *mdata){
  if (mdata == NULL) return;
  mdata->composer = NULL;
  mdata->title1 = NULL;
  mdata->title2 = NULL;
  mdata->title3 = NULL;
  mdata->tpe1 = NULL;
  mdata->tpe2 = NULL;
  mdata->tpe3 = NULL;
  mdata->tpe4 = NULL;
  mdata->date = NULL;
  mdata->year = 0;
  mdata->album = NULL;
  mdata->genre = NULL;
  mdata->duration = 0;
  mdata->partofset = 0;
  return;
}

void free_mdata(AudioMetaData *mdata){
  if (mdata == NULL) return;
  if (mdata->composer) free(mdata->composer);
  if (mdata->title1) free(mdata->title1);
  if (mdata->title2) free(mdata->title2);
  if (mdata->title3) free(mdata->title3);
  if (mdata->tpe1) free(mdata->tpe1);
  if (mdata->tpe2) free(mdata->tpe2);
  if (mdata->tpe3) free(mdata->tpe3);
  if (mdata->tpe4) free(mdata->tpe4);
  if (mdata->date) free(mdata->date);
  if (mdata->album) free(mdata->album);
  if (mdata->genre) free(mdata->genre);
  init_mdata(mdata);
  return;
}

#ifdef HAVE_MPG123

static void get_v1_data(mpg123_id3v1 *v1, AudioMetaData *mdata){

  char tmp[64];
  memcpy(tmp, v1->title, sizeof(v1->title));
  tmp[sizeof(v1->title)] = '\0';
  mdata->title2 = (char*)strdup(tmp);

  memcpy(tmp, v1->artist, sizeof(v1->artist));
  tmp[sizeof(v1->artist)] = '\0';
  mdata->composer = strdup(tmp);

  memcpy(tmp, v1->album, sizeof(v1->album));
  tmp[sizeof(v1->album)] = '\0';
  mdata->album = strdup(tmp);

  memcpy(tmp, v1->year, sizeof(v1->year));
  tmp[sizeof(v1->year)] = '\0';
  mdata->year = atoi(tmp);

  snprintf(tmp, 64, "%d", v1->genre);
  mdata->genre = strdup(tmp);
  return;
}

static void get_lines(mpg123_string *inlines, char **str){
  char *lines = NULL;
  size_t len = 0;
  if (inlines !=  NULL && inlines->fill){
    lines = inlines->p;
    len = inlines->fill;
  } else {
    *str = NULL;
    return;
  }

  *str = (char*)malloc((len+1)*sizeof(char));
  memcpy(*str, lines, len);
  (*str)[len] = '\0';
  size_t index = 0;
  while (index < len){
      if (lines[index] == '\0' || lines[index] == '\r' || lines[index] == '\n'){
	  (*str)[index] = 0x20;
      }
      index++;
  }
  return;
}

void get_v2_data(mpg123_id3v2 *v2, AudioMetaData *mdata){

  get_lines(v2->artist, &mdata->composer);
  get_lines(v2->album, &mdata->album);
  //get_lines(v2->title, &mdata->title2);

  char *year = NULL;
  get_lines(v2->year, &year);
  if (year)mdata->year = (int)atoi(year);
  if (year != NULL) free(year);

  get_lines(v2->genre, &mdata->genre);

  char id[5];
  id[4] = '\0';
  size_t i;
  for (i = 0; i < v2->texts; i++){
      if (v2->text[i].id != NULL){
	  memcpy(id, v2->text[i].id, 4);
	  if (!strcmp(id, "TDAT")){
	      get_lines(&v2->text[i].text, &mdata->date);
	  } else if (!strcmp(id, "TLEN")){
	      char *duration;
	      get_lines(&v2->text[i].text, &duration);
	      mdata->duration = atoi(duration);
	  } else if (!strcmp(id, "TIT1")){
	      get_lines(&v2->text[i].text, &mdata->title1);
	  } else if (!strcmp(id, "TIT2")){
	      if (mdata->title2 == NULL) {
		  get_lines(&v2->text[i].text, &mdata->title2);
	      }
	  } else if (!strcmp(id, "TIT3")){ 
	      get_lines(&v2->text[i].text, &mdata->title3);
	  } else if (!strcmp(id, "TPE1")){
	      get_lines(&v2->text[i].text, &mdata->tpe1);
	  } else if (!strcmp(id, "TPE2")){
	      get_lines(&v2->text[i].text,&mdata->tpe2);
	  } else if (!strcmp(id, "TPE3")){
	      get_lines(&v2->text[i].text,&mdata->tpe3);
	  } else if (!strcmp(id, "TPOS")){
	      char *pos;
	      get_lines(&v2->text[i].text, &pos);
	      mdata->partofset = atoi(pos);
	  }
      }
  }
  return;
}

float* readaudio_mp3(const char *filename,long *sr, unsigned int *buflen,\
		     const float nbsecs, AudioMetaData *mdata, int *error){
  mpg123_handle *m;
  int ret = 0;
  mpg123_id3v1 *v1 = NULL;
  mpg123_id3v2 *v2 = NULL;

  if ((ret = mpg123_init()) != MPG123_OK || ((m = mpg123_new(NULL,&ret)) == NULL)|| \
      (ret = mpg123_open(m, filename)) != MPG123_OK){
    *error = (ret != 0) ? ret : PHERR_MP3NEW;
    return NULL;
  }

  /*turn off logging */
  mpg123_param(m, MPG123_ADD_FLAGS, MPG123_QUIET, 0);

  off_t totalsamples;
  
  mpg123_scan(m);
  totalsamples = mpg123_length(m);
  if (totalsamples <= 0){
    *error = PHERR_NOSAMPLES;
    return NULL;
  }
  
  int meta = mpg123_meta_check(m);

  if (mdata)init_mdata(mdata);
  if (mdata && (meta & MPG123_ID3) && mpg123_id3(m, &v1, &v2) == MPG123_OK){
    if (v2 != NULL){
      get_v2_data(v2, mdata);
    } else if (v1 != NULL){
      get_v1_data(v1, mdata);
    } 
  }

  int channels, encoding;
    
  if (mpg123_getformat(m, sr, &channels, &encoding) != MPG123_OK){
    *error = PHERR_NOFORMAT;
    return NULL;
  }
  
  mpg123_format_none(m);
  mpg123_format(m, *sr, channels, encoding);
  if (channels <= 0 || encoding <= 0){
    *error = PHERR_NOENCODING;
    return NULL;
  }


  size_t decbuflen = mpg123_outblock(m);
  if (decbuflen == 0){
    /* take a guess */ 
    decbuflen = 1<<16;
  }

  unsigned char *decbuf = (unsigned char*)malloc(decbuflen);  
  if (decbuf == NULL){
    *error = PHERR_MEMALLOC;
    return NULL;
  }

  unsigned int nbsamples = (nbsecs <= 0) ? totalsamples : nbsecs*(*sr);
  nbsamples = (nbsamples <= totalsamples) ? nbsamples : totalsamples;

  size_t i, j, index = 0, done;


  float *buffer = (float*)malloc(nbsamples*sizeof(float));
  if (buffer == NULL){
    *error = PHERR_MEMALLOC;
    return NULL;
  }
  *buflen = nbsamples;

  do {
    ret = mpg123_read(m, decbuf, decbuflen, &done);
    switch (encoding) {
    case MPG123_ENC_SIGNED_16 :
      for (i = 0; i < done/sizeof(short); i+=channels){
	buffer[index] = 0.0f;
	for (j = 0; j < channels ; j++){
	  buffer[index] += (float)(((short*)decbuf)[i+j])/(float)SHRT_MAX;
	}
	buffer[index++] /= channels;
	if (index >= nbsamples) break;
      }
      break;
    case MPG123_ENC_SIGNED_8:
      for (i = 0; i < done/sizeof(char); i+=channels){
	buffer[index] = 0.0f;
	for (j = 0; j < channels ; j++){
	  buffer[index] += (float)(((char*)decbuf)[i+j])/(float)SCHAR_MAX;
	}
	buffer[index++] /= channels;
	if (index >= nbsamples) break;
      }
      break;
    case MPG123_ENC_FLOAT_32:
      for (i = 0; i < done/sizeof(float); i+=channels){
	buffer[index] = 0.0f;
	for (j = 0; j < channels; j++){
	  buffer[index] += ((float*)decbuf)[i+j];
	}
	buffer[index++] /= channels;
	if (index >= nbsamples) break;
      }
      break;
    default:
	done = 0;
    }

  } while (ret == MPG123_OK && index < nbsamples);

  if (ret != MPG123_DONE && ret != MPG123_OK && index < nbsamples){
    free(buffer);
    *error = ret;
    buffer=NULL;
  }
  free(decbuf);
  mpg123_close(m);
  mpg123_delete(m);
  mpg123_exit();

  return buffer;
}
#endif /*HAVE_MPG123*/

#ifdef HAVE_AMR

const int sizes[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0 };

static
int count_amr_samples(const char *file){
    char header[6];
    size_t n;
    int amrfd = open(file, O_RDONLY, S_IRUSR);
    if (amrfd < 0)return -1;
    
    n = read(amrfd, header, 6);
    if (n != 6 || memcmp(header, "#!AMR\n", 6)){
	close(amrfd);
	return -1;
    }

    uint8_t buffer[500];
    int size, index = 0;
    while (1){
	n = read(amrfd, buffer, 1);
	if (n <= 0) break;
	size = sizes[(buffer[0] >> 3) & 0x0f];
	n = read(amrfd, buffer + 1, size);
	if (n != size) break;
	index += 160;
    }

    close(amrfd);
    return index;
}

static
float* readaudio_amr(const char *file, long *sr, unsigned int *buflen, 
                     const float nbsecs, AudioMetaData *mdata,int *error){
    char header[6];
    size_t n;
    void *amr;
    *buflen = 0;
    *sr = 8000;
    int nbsamples = count_amr_samples(file);
    if (nbsamples <= 0) return NULL;

    nbsamples = (nbsecs <= 0.0f) ? nbsamples : 8000*(int)nbsecs;
    
    float *buf = malloc(nbsamples*sizeof(float));
    if (buf == NULL){
	return NULL;
    }

    int amrfd = open(file, O_RDONLY, S_IRUSR);
    if (amrfd < 0){
	free(buf);
	return NULL;
    }

    n = read(amrfd, header, 6);
    if (n != 6 || memcmp(header, "#!AMR\n", 6)){
	close(amrfd);
	free(buf);
    }

    int16_t outbuffer[160];
    uint8_t buffer[500];
    int size, i, index = 0;
    amr = Decoder_Interface_init();
    while (1){
	n = read(amrfd, buffer, 1);
	if (n <= 0) break;

	size = sizes[(buffer[0] >> 3) & 0x0f];

	n = read(amrfd, buffer + 1, size);
	if (n != size) break;

	/* decode packet */
	Decoder_Interface_Decode(amr, buffer, outbuffer, 0);

	for (i=0;i<160;i++){
	    buf[index++] = (float)outbuffer[i]/(float)SHRT_MAX;
	}

	if (index >= nbsamples) break;
    }

    *buflen = index;
    Decoder_Interface_exit(amr);
    close(amrfd);

    return buf;
}

#endif /* HAVE_AMR */

static
float *readaudio_snd(const char *filename, long *sr, unsigned int *buflen,\
		     const float nbsecs, AudioMetaData *mdata, int *error){

    SF_INFO sf_info;
    SNDFILE *sndfile;
    sf_count_t cnt_frames;
    const char *tmp;
    float *inbuf, *buf;
    unsigned int src_frames;
    int i,j,indx;

    sf_info.format=0;
    sndfile = sf_open(filename, SFM_READ, &sf_info);
    if (sndfile == NULL){
      *error = PHERR_SNDFILEOPEN;
      return NULL;
    }
    
    /* normalize */ 
    sf_command(sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);

    if (mdata){
	init_mdata(mdata);
	/* extract metadata from file */ 
	tmp = sf_get_string(sndfile, SF_STR_TITLE);
	mdata->title2 = (tmp) ? strdup(tmp): NULL;
      
	tmp = sf_get_string(sndfile,SF_STR_ARTIST);
	mdata->tpe1= (tmp) ? strdup(tmp) : NULL;
	mdata->composer = (tmp) ? strdup(tmp) : NULL;
      
	tmp = sf_get_string(sndfile,SF_STR_DATE);
	mdata->date = (tmp) ? strdup(tmp):NULL;
    } 

    *sr = (long)sf_info.samplerate;

    /*allocate input buffer for signal*/
    src_frames = (nbsecs <= 0) ? (unsigned int)sf_info.frames : (unsigned int)(nbsecs*sf_info.samplerate);
    src_frames = (sf_info.frames < src_frames) ? (unsigned int)sf_info.frames : src_frames;
    inbuf = (float*)malloc(src_frames*sf_info.channels*sizeof(float));
    if (inbuf == NULL){
      *error = PHERR_MEMALLOC;
      return NULL;
    }
    /*read frames */ 
    cnt_frames = sf_readf_float(sndfile, inbuf, src_frames);

    buf = (float*)malloc((size_t)cnt_frames*sizeof(float));
    if (buf == NULL){
      *error = PHERR_MEMALLOC;
      return NULL;
    }
    *buflen = (unsigned int)cnt_frames;
      
    
    /*average across all channels*/
    indx=0;
    for (i=0;i<cnt_frames*sf_info.channels;i+=sf_info.channels){
	buf[indx] = 0;
	for (j=0;j<sf_info.channels;j++){
	    buf[indx] += inbuf[i+j];
	}
	buf[indx++] /= sf_info.channels;
    }
    free(inbuf);
    sf_close(sndfile);

    return buf;
}


float* readaudio(const char *filename, const int sr, const float nbsecs, float *sigbuf, int *buflen,
                                         AudioMetaData *mdata, int *error){
  SRC_STATE *src_state = NULL;
  SRC_DATA src_data;
  long orig_sr;
  unsigned int orig_length = 0, outbufferlength = 0;
  char *suffix = NULL, *name = NULL;
  float *inbuffer = NULL, *outbuffer = NULL;
  double sr_ratio = 1.0f;
  *error = PHERR_SUCCESS;

  if (filename == NULL || buflen == NULL) {
    *error = PHERR_NULLARG;
    return NULL;
  }

  if (mdata) init_mdata(mdata);

  suffix = strrchr(filename, '.');

  if (*suffix != '\0' && (!strncasecmp(suffix+1, "mp3",3) || !strncasecmp(suffix+1, "mp2", 3))) {

#ifdef HAVE_MPG123
    inbuffer = readaudio_mp3(filename, &orig_sr, &orig_length, nbsecs, mdata, error);
#else
    *error = PHERR_NOFORMAT;
    return NULL;
#endif

  } else if (*suffix != '\0' && !strncasecmp(suffix+1, "amr", 3)) {

#ifdef HAVE_AMR
      inbuffer = readaudio_amr(filename, &orig_sr, &orig_length, nbsecs, mdata, error);
#else
      *error = PHERR_NOFORMAT;
      return NULL;
#endif

  } else {
    inbuffer = readaudio_snd(filename, &orig_sr, &orig_length, nbsecs, mdata, error);
  }  

  if (inbuffer == NULL) {
      *error = PHERR_NOBUF;
      return NULL;
  }

  /* if no data extracted for title, use the file name */ 
  if (mdata && mdata->title2 == NULL){
      name = strrchr(filename, '/');
      if (name == NULL) name = strchr(filename, '\\');
      if (name) mdata->title2 = strdup(name+1);
  }

  /* resample float array */ 
  /* set desired sr ratio */ 
  sr_ratio = (double)(sr)/(double)orig_sr;
  if (src_is_valid_ratio(sr_ratio) == 0){
    *error = PHERR_BADSR;
    free(inbuffer);
    return NULL;
  }

  /* allocate output buffer for conversion */ 
  outbufferlength = (unsigned int)(sr_ratio*orig_length);

  outbuffer = NULL;
  if (sigbuf && outbufferlength < *buflen){
    outbuffer = sigbuf;
  } else {
    outbuffer = (float*)malloc(outbufferlength*sizeof(float));
  }

  if (!outbuffer){
    free(inbuffer);
    *error = PHERR_NOBUFALLOCD;
    return NULL;
  }

  src_state = src_new(SRC_SINC_FASTEST, 1, error);
  if (!src_state){
    *error = PHERR_SRCCONTXT;
    free(inbuffer);
    if (outbuffer != sigbuf) free(outbuffer);
     return NULL;
  }


  src_data.data_in = inbuffer;
  src_data.data_out = outbuffer;
  src_data.input_frames = orig_length;
  src_data.output_frames = outbufferlength;
  src_data.end_of_input = SF_TRUE;
  src_data.src_ratio = sr_ratio;

  /* sample rate conversion */ 
  *error = src_process(src_state, &src_data);
  if (*error){
    *error = PHERR_SRCPROC;
    free(inbuffer);
    if (outbuffer != sigbuf) free(outbuffer);
    src_delete(src_state);
    return NULL;
  }

  *buflen = outbufferlength;

  src_delete(src_state);
  free(inbuffer);

  return outbuffer;
} 


void freebuffer(void *ptr){
    free(ptr);
    return;
}
