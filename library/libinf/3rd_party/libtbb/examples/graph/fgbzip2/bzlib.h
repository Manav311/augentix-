/*
    Copyright 2005-2016 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks. Threading Building Blocks is free software;
    you can redistribute it and/or modify it under the terms of the GNU General Public License
    version 2  as  published  by  the  Free Software Foundation.  Threading Building Blocks is
    distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See  the GNU General Public License for more details.   You should have received a copy of
    the  GNU General Public License along with Threading Building Blocks; if not, write to the
    Free Software Foundation, Inc.,  51 Franklin St,  Fifth Floor,  Boston,  MA 02110-1301 USA

    As a special exception,  you may use this file  as part of a free software library without
    restriction.  Specifically,  if other files instantiate templates  or use macros or inline
    functions from this file, or you compile this file and link it with other files to produce
    an executable,  this file does not by itself cause the resulting executable to be covered
    by the GNU General Public License. This exception does not however invalidate any other
    reasons why the executable file might be covered by the GNU General Public License.
*/

/*-------------------------------------------------------------*/
/*--- Public header file for the library.                   ---*/
/*---                                               bzlib.h ---*/
/*-------------------------------------------------------------*/

/* ------------------------------------------------------------------
   The original source for this example:
   This file is part of bzip2/libbzip2, a program and library for
   lossless, block-sorting data compression.

   bzip2/libbzip2 version 1.0.6 of 6 September 2010
   Copyright (C) 1996-2010 Julian Seward <jseward@bzip.org>

   This program, "bzip2", the associated library "libbzip2", and all
   documentation, are copyright (C) 1996-2010 Julian R Seward.  All
   rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   2. The origin of this software must not be misrepresented; you must 
   not claim that you wrote the original software.  If you use this 
   software in a product, an acknowledgment in the product 
   documentation would be appreciated but is not required.

   3. Altered source versions must be plainly marked as such, and must
   not be misrepresented as being the original software.

   4. The name of the author may not be used to endorse or promote 
   products derived from this software without specific prior written 
   permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Julian Seward, jseward@bzip.org
   bzip2/libbzip2 version 1.0.6 of 6 September 2010
   ------------------------------------------------------------------ */

#ifndef _BZLIB_H
#define _BZLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define BZ_RUN 0
#define BZ_FLUSH 1
#define BZ_FINISH 2

#define BZ_OK 0
#define BZ_RUN_OK 1
#define BZ_FLUSH_OK 2
#define BZ_FINISH_OK 3
#define BZ_STREAM_END 4
#define BZ_SEQUENCE_ERROR (-1)
#define BZ_PARAM_ERROR (-2)
#define BZ_MEM_ERROR (-3)
#define BZ_DATA_ERROR (-4)
#define BZ_DATA_ERROR_MAGIC (-5)
#define BZ_IO_ERROR (-6)
#define BZ_UNEXPECTED_EOF (-7)
#define BZ_OUTBUFF_FULL (-8)
#define BZ_CONFIG_ERROR (-9)

typedef struct {
	char *next_in;
	unsigned int avail_in;
	unsigned int total_in_lo32;
	unsigned int total_in_hi32;

	char *next_out;
	unsigned int avail_out;
	unsigned int total_out_lo32;
	unsigned int total_out_hi32;

	void *state;

	void *(*bzalloc)(void *, int, int);
	void (*bzfree)(void *, void *);
	void *opaque;
} bz_stream;

#ifndef BZ_IMPORT
#define BZ_EXPORT
#endif

#ifndef BZ_NO_STDIO
/* Need a definitition for FILE */
#include <stdio.h>
#endif

#ifdef _WIN32
#include <windows.h>
#ifdef small
/* windows.h define small to char */
#undef small
#endif
#ifdef BZ_EXPORT
#define BZ_API(func) WINAPI func
#define BZ_EXTERN extern
#else
/* import windows dll dynamically */
#define BZ_API(func) (WINAPI * func)
#define BZ_EXTERN
#endif
#else
#define BZ_API(func) func
#define BZ_EXTERN extern
#endif

/*-- Core (low-level) library functions --*/

BZ_EXTERN int BZ_API(BZ2_bzCompressInit)(bz_stream *strm, int blockSize100k, int verbosity, int workFactor);

BZ_EXTERN int BZ_API(BZ2_bzCompress)(bz_stream *strm, int action);

BZ_EXTERN int BZ_API(BZ2_bzCompressEnd)(bz_stream *strm);

BZ_EXTERN int BZ_API(BZ2_bzDecompressInit)(bz_stream *strm, int verbosity, int small);

BZ_EXTERN int BZ_API(BZ2_bzDecompress)(bz_stream *strm);

BZ_EXTERN int BZ_API(BZ2_bzDecompressEnd)(bz_stream *strm);

/*-- High(er) level library functions --*/

#ifndef BZ_NO_STDIO
#define BZ_MAX_UNUSED 5000

typedef void BZFILE;

BZ_EXTERN BZFILE *BZ_API(BZ2_bzReadOpen)(int *bzerror, FILE *f, int verbosity, int small, void *unused, int nUnused);

BZ_EXTERN void BZ_API(BZ2_bzReadClose)(int *bzerror, BZFILE *b);

BZ_EXTERN void BZ_API(BZ2_bzReadGetUnused)(int *bzerror, BZFILE *b, void **unused, int *nUnused);

BZ_EXTERN int BZ_API(BZ2_bzRead)(int *bzerror, BZFILE *b, void *buf, int len);

BZ_EXTERN BZFILE *BZ_API(BZ2_bzWriteOpen)(int *bzerror, FILE *f, int blockSize100k, int verbosity, int workFactor);

BZ_EXTERN void BZ_API(BZ2_bzWrite)(int *bzerror, BZFILE *b, void *buf, int len);

BZ_EXTERN void BZ_API(BZ2_bzWriteClose)(int *bzerror, BZFILE *b, int abandon, unsigned int *nbytes_in,
                                        unsigned int *nbytes_out);

BZ_EXTERN void BZ_API(BZ2_bzWriteClose64)(int *bzerror, BZFILE *b, int abandon, unsigned int *nbytes_in_lo32,
                                          unsigned int *nbytes_in_hi32, unsigned int *nbytes_out_lo32,
                                          unsigned int *nbytes_out_hi32);
#endif

/*-- Utility functions --*/

BZ_EXTERN int BZ_API(BZ2_bzBuffToBuffCompress)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen,
                                               int blockSize100k, int verbosity, int workFactor);

BZ_EXTERN int BZ_API(BZ2_bzBuffToBuffDecompress)(char *dest, unsigned int *destLen, char *source,
                                                 unsigned int sourceLen, int small, int verbosity);

/*--
   Code contributed by Yoshioka Tsuneo (tsuneo@rr.iij4u.or.jp)
   to support better zlib compatibility.
   This code is not _officially_ part of libbzip2 (yet);
   I haven't tested it, documented it, or considered the
   threading-safeness of it.
   If this code breaks, please contact both Yoshioka and me.
--*/

BZ_EXTERN const char *BZ_API(BZ2_bzlibVersion)(void);

#ifndef BZ_NO_STDIO
BZ_EXTERN BZFILE *BZ_API(BZ2_bzopen)(const char *path, const char *mode);

BZ_EXTERN BZFILE *BZ_API(BZ2_bzdopen)(int fd, const char *mode);

BZ_EXTERN int BZ_API(BZ2_bzread)(BZFILE *b, void *buf, int len);

BZ_EXTERN int BZ_API(BZ2_bzwrite)(BZFILE *b, void *buf, int len);

BZ_EXTERN int BZ_API(BZ2_bzflush)(BZFILE *b);

BZ_EXTERN void BZ_API(BZ2_bzclose)(BZFILE *b);

BZ_EXTERN const char *BZ_API(BZ2_bzerror)(BZFILE *b, int *errnum);
#endif

#ifdef __cplusplus
}
#endif

#endif

/*-------------------------------------------------------------*/
/*--- end                                           bzlib.h ---*/
/*-------------------------------------------------------------*/
