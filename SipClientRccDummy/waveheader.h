/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* the following code was taken from a free software utility that I don't remember the name. */
/* sorry */

#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#ifndef waveheader_h
#define waveheader_h

//typedef unsigned __int8 uint8_t;
//typedef unsigned __int16 uint16_t;
//typedef unsigned __int32 uint32_t;
//typedef unsigned __int64 uint64_t;
//typedef __int8 int8_t;
//typedef __int16 int16_t;
//typedef __int32 int32_t;
//typedef __int64 int64_t;
//typedef unsigned long in_addr_t;
/* all integer in wav header must be read in least endian order */
static inline unsigned __int16 swap16(unsigned __int16 a)
{
	return ((a & 0xFF) << 8) | ((a & 0xFF00) >> 8);
}

static inline unsigned __int32 swap32(unsigned __int32 a)
{
	return ((a & 0xFF) << 24) | ((a & 0xFF00) << 8) | 
		((a & 0xFF0000) >> 8) | ((a & 0xFF000000) >> 24);
}

#ifdef WORDS_BIGENDIAN
#define le_uint32(a) (swap32((a)))
#define le_uint16(a) (swap16((a)))
#define le_int16(a) ( (int16_t) swap16((uint16_t)((a))) )
#else
#define le_uint32(a) (a)
#define le_uint16(a) (a)
#define le_int16(a) (a)
#endif

typedef struct _riff_t {
	char riff[4] ;	/* "RIFF" (ASCII characters) */
	unsigned __int32  len ;	/* Length of package (binary, little endian) */
	char wave[4] ;	/* "WAVE" (ASCII characters) */
} riff_t;

/* The FORMAT chunk */

typedef struct _format_t {
	char  fmt[4] ;		/* "fmt_" (ASCII characters) */
	unsigned __int32   len ;	/* length of FORMAT chunk (always 0x10) */
	unsigned __int16  type;		/* codec type*/
	unsigned __int16 channel ;	/* Channel numbers (0x01 = mono, 0x02 = stereo) */
	unsigned __int32   rate ;	/* Sample rate (binary, in Hz) */
	unsigned __int32   bps ;	/* Average Bytes Per Second */
	unsigned __int16 blockalign ;	/*number of bytes per sample */
	unsigned __int16 bitpspl ;	/* bits per sample */
} format_t;

/* The DATA chunk */

typedef struct _data_t {
	char data[4] ;	/* "data" (ASCII characters) */
	int  len ;	/* length of data */
} data_t;

typedef struct _wave_header_t
{
	riff_t riff_chunk;
	format_t format_chunk;
	data_t data_chunk;
} wave_header_t;


#define wave_header_get_rate(header)		le_uint32((header)->format_chunk.rate)
#define wave_header_get_channel(header)		le_uint16((header)->format_chunk.channel)
#define wave_header_get_bpsmpl(header) \
	le_uint16((header)->format_chunk.blockalign)
#endif
