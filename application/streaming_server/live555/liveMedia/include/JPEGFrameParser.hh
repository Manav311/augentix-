/*
 *  Copyright (C) Peter Gaal
 *  this code is derived from work of W.L. Chuang <ponponli2000 at gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _JPEG_FRAME_PARSER_HH_INCLUDED
#define _JPEG_FRAME_PARSER_HH_INCLUDED

class JPEGFrameParser {
    public:
	JPEGFrameParser();
	virtual ~JPEGFrameParser();

	unsigned char width()
	{
		return _width;
	}
	unsigned char height()
	{
		return _height;
	}
	unsigned char type()
	{
		return _type;
	}
	unsigned char precision()
	{
		return _precision;
	}
	unsigned char qFactor()
	{
		return _qFactor;
	}

	unsigned short restartInterval()
	{
		return _restartInterval;
	}

	unsigned char const *quantizationTables(unsigned short &length)
	{
		length = _qTablesLength;
		return _qTables;
	}

	int parse(unsigned char *data, unsigned int size);

	unsigned char const *scandata(unsigned int &length)
	{
		length = _scandataLength;

		return _scandata;
	}

    private:
	unsigned int scanJpegMarker(const unsigned char *data, unsigned int size, unsigned int *offset);
	int readSOF(const unsigned char *data, unsigned int size, unsigned int *offset);
	unsigned int readDQT(const unsigned char *data, unsigned int size, unsigned int offset);
	int readDRI(const unsigned char *data, unsigned int size, unsigned int *offset);

    private:
	unsigned char _width;
	unsigned char _height;
	unsigned char _type;
	unsigned char _precision;
	unsigned char _qFactor;

	unsigned char *_qTables;
	unsigned short _qTablesLength;

	unsigned short _restartInterval;

	unsigned char *_scandata;
	unsigned int _scandataLength;
};

#endif /* _JPEG_FRAME_PARSER_HH_INCLUDED */
