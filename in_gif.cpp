/*
 * in_gif.cpp -- read a Compuserve GIF file
 * by pts@fazekas.hu at Fri Mar  1 22:28:46 CET 2002
 *
 */

/*The creators of the GIF format require the following
  acknowledgement:
  The Graphics Interchange Format(c) is the Copyright property of
  CompuServe Incorporated. GIF(sm) is a Service Mark property of
  CompuServe Incorporated.
*/         

#ifdef __GNUC__
#pragma implementation
#endif

#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 2

#include "config2.h" /* SUXX, ignores features.h */

#if USE_IN_GIF

#include "cgif.h"
#include "cgif.c" /* _POSIX_SOURCE */
#include "image.hpp"
#include "error.hpp"

#undef CGIFFF
#define CGIFFF CGIF::

static Image::Sampled *in_gif_reader(Image::filep_t file_, SimBuffer::Flat const&) {
  Image::Indexed *img;
  // Error::sev(Error::ERROR) << "Cannot load XPM images yet." << (Error*)0;
  CGIFFF GifFileType *giff;
  CGIFFF SavedImage *sp;
  CGIFFF ColorMapObject *cm;
  
  if (0==(giff=CGIFFF DGifOpenFILE(file_)) || GIF_ERROR==CGIFFF DGifSlurp(giff))
    Error::sev(Error::ERROR) << "GIF: " << (CGIFFF GetGifError() || "unknown error") << (Error*)0;
  if (giff->ImageCount<1)
    Error::sev(Error::ERROR) << "GIF: no image in file" << (Error*)0;

  sp=giff->SavedImages+0;
  cm = (sp->ImageDesc.ColorMap ? sp->ImageDesc.ColorMap : giff->SColorMap);
  img=new Image::Indexed(sp->ImageDesc.Width, sp->ImageDesc.Height, cm->ColorCount, 8);
  CGIFFF GifColorType *co=cm->Colors, *ce=co+cm->ColorCount;
  char *p=img->getHeadp();
  while (co!=ce) { *p++=(char)co->Red; *p++=(char)co->Green; *p++=(char)co->Blue; co++; }
  // fprintf(stderr, "transp=%d\n", sp->transp);
  if (sp->transp!=-1) img->setTransp(sp->transp);
  /* ^^^ comment out this line to ignore transparency of the GIF file */
  
  assert(1L*sp->ImageDesc.Width*sp->ImageDesc.Height<=img->end_()-img->getRowbeg());
  memcpy(img->getRowbeg(), sp->RasterBits, (slen_t)sp->ImageDesc.Width*sp->ImageDesc.Height);

  CGIFFF DGifCloseFile(giff); /* also frees memory structure */
  
  return img;
}

static Image::Loader::reader_t in_gif_checker(char buf[Image::Loader::MAGIC_LEN], char [Image::Loader::MAGIC_LEN], SimBuffer::Flat const&) {
  return (0==memcmp(buf,"GIF87a",6) || 0==memcmp(buf,"GIF89a",6)) ? in_gif_reader : 0;
}

#define in_gif_name "GIF"

#else
#include "image.hpp"
#include "error.hpp"
#define in_gif_name (char const*)NULLP
/* #define in_gif_checker (Image::Loader::checker_t)NULLP */
static Image::Loader::reader_t in_gif_checker(char buf[Image::Loader::MAGIC_LEN], char [Image::Loader::MAGIC_LEN], SimBuffer::Flat const&) {
  if (0==memcmp(buf,"GIF87a",6) || 0==memcmp(buf,"GIF89a",6)) {
    Error::sev(Error::WARNING) << "loader: please `configure --enable-gif' for loading GIF files" << (Error*)0;
  }
  return 0;
}
#endif /* USE_IN_GIF */

Image::Loader in_gif_loader = { in_gif_name, in_gif_checker, 0 };