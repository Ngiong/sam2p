/*
 * in_pnm.cpp -- read a NetPBM PNM bitmap file
 * by pts@fazekas.hu at Sat Mar  2 00:46:54 CET 2002
 *
 */

#ifdef __GNUC__
#pragma implementation
#endif

#include "image.hpp"
#include "error.hpp"

#if USE_IN_PNM

#include "input-pnm.ci"
#include <string.h>
#include <stdio.h>

static Image::Sampled *in_pnm_reader(Image::filep_t file_, SimBuffer::Flat const&) {
  FILE *f=(FILE*)file_;
  Image::Sampled *ret=0;
  bitmap_type bitmap=pnm_load_image((char*)(void*)f);
  /* Imp: Work without duplicated memory allocation */
  if (BITMAP_PLANES(bitmap)==1) {
    Image::Gray *img=new Image::Gray(BITMAP_WIDTH(bitmap), BITMAP_HEIGHT(bitmap), 8);
    memcpy(img->getRowbeg(), BITMAP_BITS(bitmap), (slen_t)BITMAP_WIDTH(bitmap)*BITMAP_HEIGHT(bitmap));
    ret=img;
  } else if (BITMAP_PLANES(bitmap)==3) {
    Image::RGB *img=new Image::RGB(BITMAP_WIDTH(bitmap), BITMAP_HEIGHT(bitmap), 8);
    memcpy(img->getRowbeg(), BITMAP_BITS(bitmap), (slen_t)3*BITMAP_WIDTH(bitmap)*BITMAP_HEIGHT(bitmap));
    /* fwrite(BITMAP_BITS(bitmap), 1, (slen_t)3*BITMAP_WIDTH(bitmap)*BITMAP_HEIGHT(bitmap), stdout); */
    ret=img;
  } else assert(0 && "invalid PNM depth");
  delete [] BITMAP_BITS(bitmap);
  if (MACRO_GETC(f)=='P') {
    ungetc('P',f);
    // Error::sev(Error::NOTICE) << "PNM: loading alpha after PNM: "
    //   " ftell=" << ftell(f) <<
    //  " bytes=" << ((unsigned)bitmap.width*bitmap.height*bitmap.np)  << (Error*)0;
    bitmap=pnm_load_image((char*)(void*)f); 
    // fwrite(bitmap.bitmap,1,(unsigned)bitmap.width*bitmap.height*bitmap.np,stdout);
    /* Dat: black pixel is transparent */
    if (BITMAP_PLANES(bitmap)!=1) Error::sev(Error::ERROR) << "PNM: alpha must be PBM or PGM" << (Error*)0;
    Image::Gray *img=new Image::Gray(BITMAP_WIDTH(bitmap), BITMAP_HEIGHT(bitmap), 8);
    memcpy(img->getRowbeg(), BITMAP_BITS(bitmap), (slen_t)BITMAP_WIDTH(bitmap)*BITMAP_HEIGHT(bitmap));
    delete [] BITMAP_BITS(bitmap);
    Image::Sampled *old=ret;
    // old->packPal(); /* automatically called */
    if ((ret=old->addAlpha(img))!=old) {
      Error::sev(Error::NOTICE) << "PNM: loaded alpha after PNM" << (Error*)0;
      delete old;
    } else {
      Error::sev(Error::NOTICE) << "PNM: loaded alpha, but no transparent pixels" << (Error*)0;
    }
  }
  /* fclose(f); */
  return ret;
}

static Image::Loader::reader_t in_pnm_checker(char buf[Image::Loader::MAGIC_LEN], char [Image::Loader::MAGIC_LEN], SimBuffer::Flat const&) {
  return (buf[0]=='P' && (buf[1]>='1' && buf[1]<='6') &&
    (buf[2]=='\t' || buf[2]==' ' || buf[2]=='\r' || buf[2]=='\n' || buf[2]=='#'))
   ? in_pnm_reader : 0;
}

#else
#define in_pnm_checker NULLP
#endif /* USE_IN_XPM */

Image::Loader in_pnm_loader = { "PNM", in_pnm_checker, 0 };
