/*
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use.  Users may copy or modify this source code without
 * charge.
 *
 * SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
 * THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/* 16kbps version created, used 24kbps code and changing as little as possible.
 * G.726 specs are available from ITU's gopher or WWW site (http://www.itu.ch)
 * If any errors are found, please contact me at mrand@tamu.edu
 *      -Marc Randolph
 */

/*
 * g726_16.c
 *
 * Description:
 *
 * g723_16_encoder(), g723_16_decoder()
 *
 * These routines comprise an implementation of the CCITT G.726 16 Kbps
 * ADPCM coding algorithm.  Essentially, this implementation is identical to
 * the bit level description except for a few deviations which take advantage
 * of workstation attributes, such as hardware 2's complement arithmetic.
 *
 * The ITU-T G.726 coder is an adaptive differential pulse code modulation
 * (ADPCM) waveform coding algorithm, suitable for coding of digitized
 * telephone bandwidth (0.3-3.4 kHz) speech or audio signals sampled at 8 kHz.
 * This coder operates on a sample-by-sample basis. Input samples may be
 * represented in linear PCM or companded 8-bit G.711 (m-law/A-law) formats
 * (i.e., 64 kbps). For 32 kbps operation, each sample is converted into a
 * 4-bit quantized difference signal resulting in a compression ratio of
 * 2:1 over the G.711 format. For 24 kbps 40 kbps operation, the quantized
 * difference signal is 3 bits and 5 bits, respectively.
 *
 * $Log: g726_16.c,v $
 * Revision 1.4  2002/11/20 04:29:13  robertj
 * Included optimisations for G.711 and G.726 codecs, thanks Ted Szoczei
 *
 * Revision 1.1  2002/02/11 23:24:23  robertj
 * Updated to openH323 v1.8.0
 *
 * Revision 1.2  2002/02/10 21:14:54  dereks
 * Add cvs log history to head of the file.
 * Ensure file is terminated by a newline.
 *
 *
 *
 *
 */
#include "g72x.h"
#include "private.h"

/*
 * Maps G.723_16 code word to reconstructed scale factor normalized log
 * magnitude values.  Comes from Table 11/G.726
 */
static short	_dqlntab[4] = { 116, 365, 365, 116};

/* Maps G.723_16 code word to log of scale factor multiplier.
 *
 * _witab[4] is actually {-22 , 439, 439, -22}, but FILTD wants it
 * as WI << 5  (multiplied by 32), so we'll do that here
 */
static short	_witab[4] = {-704, 14048, 14048, -704};

/*
 * Maps G.723_16 code words to a set of values whose long and short
 * term averages are computed and then compared to give an indication
 * how stationary (steady state) the signal is.
 */

/* Comes from FUNCTF */
static short	_fitab[4] = {0, 0xE00, 0xE00, 0};

/* Comes from quantizer decision level tables (Table 7/G.726)
 */
static int qtab_723_16[1] = {261};


/*
 * g723_16_encoder()
 *
 * Encodes a linear PCM, A-law or u-law input sample and returns its 2-bit code.
 * Returns -1 if invalid input coding value.
 */
int
g726_16_encoder(
	int		sl,
	int		in_coding,
	g726_state *state_ptr)
{
	int		sezi;
	int		sez;			/* ACCUM */
	int		sei;
	int		se;
	int		d;				/* SUBTA */
	int		y;				/* MIX */
	int		i;
	int		dq;
	int		sr;				/* ADDB */
	int		dqsez;			/* ADDC */

	switch (in_coding) {	/* linearize input sample to 14-bit PCM */
	case AUDIO_ENCODING_ALAW:
		sl = alaw2linear(sl) >> 2;
		break;
	case AUDIO_ENCODING_ULAW:
		sl = ulaw2linear(sl) >> 2;
		break;
	case AUDIO_ENCODING_LINEAR:
		sl >>= 2;		/* sl of 14-bit dynamic range */
		break;
	default:
		return (-1);
	}

	sezi = predictor_zero(state_ptr);
	sez = sezi >> 1;
	sei = sezi + predictor_pole(state_ptr);
	se = sei >> 1;			/* se = estimated signal */

	d = sl - se;			/* d = estimation diff. */

	/* quantize prediction difference d */
	y = step_size(state_ptr);	/* quantizer step size */
	i = quantize(d, y, qtab_723_16, 1);  /* i = ADPCM code */

	      /* Since quantize() only produces a three level output
	       * (1, 2, or 3), we must create the fourth one on our own
	       */
	if (i == 3)                          /* i code for the zero region */
	  if ((d & 0x8000) == 0)             /* If d > 0, i=3 isn't right... */
	    i = 0;

	dq = reconstruct(i & 2, _dqlntab[i], y); /* quantized diff. */

	sr = (dq < 0) ? se - (dq & 0x3FFF) : se + dq; /* reconstructed signal */

	dqsez = sr + sez - se;		/* pole prediction diff. */

	update(2, y, _witab[i], _fitab[i], dq, sr, dqsez, state_ptr);

	return (i);
}

/*
 * g723_16_decoder()
 *
 * Decodes a 2-bit CCITT G.723_16 ADPCM code and returns
 * the resulting 16-bit linear PCM, A-law or u-law sample value.
 * -1 is returned if the output coding is unknown.
 */
int
g726_16_decoder(
	int		i,
	int		out_coding,
	g726_state *state_ptr)
{
	int		sezi;
	int		sez;			/* ACCUM */
	int		sei;
	int		se;
	int		y;				/* MIX */
	int		dq;
	int		sr;				/* ADDB */
	int		dqsez;

	i &= 0x03;			/* mask to get proper bits */
	sezi = predictor_zero(state_ptr);
	sez = sezi >> 1;
	sei = sezi + predictor_pole(state_ptr);
	se = sei >> 1;			/* se = estimated signal */

	y = step_size(state_ptr);	/* adaptive quantizer step size */
	dq = reconstruct(i & 0x02, _dqlntab[i], y); /* unquantize pred diff */

	sr = (dq < 0) ? (se - (dq & 0x3FFF)) : (se + dq); /* reconst. signal */

	dqsez = sr - se + sez;			/* pole prediction diff. */

	update(2, y, _witab[i], _fitab[i], dq, sr, dqsez, state_ptr);

	switch (out_coding) {
	case AUDIO_ENCODING_ALAW:
		return (tandem_adjust_alaw(sr, se, y, i, 2, qtab_723_16));
	case AUDIO_ENCODING_ULAW:
		return (tandem_adjust_ulaw(sr, se, y, i, 2, qtab_723_16));
	case AUDIO_ENCODING_LINEAR:
		return (sr << 2);	/* sr was of 14-bit dynamic range */
	default:
		return (-1);
	}
}
