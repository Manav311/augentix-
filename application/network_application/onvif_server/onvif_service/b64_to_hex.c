#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b64_to_hex.h"
/*
** Translation Table as described in RFC1113
*/
static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** returnable errors
**
** Error codes returned to the operating system.
**
*/
#define B64_SYNTAX_ERROR 1
#define B64_FILE_ERROR 2
#define B64_FILE_IO_ERROR 3
#define B64_ERROR_OUT_CLOSE 4
#define B64_LINE_SIZE_TO_MIN 5
#define B64_SYNTAX_TOOMANYARGS 6

/*
** b64_message
**
** Gather text messages in one place.
**
*/
#define B64_MAX_MESSAGES 7
static char *b64_msgs[B64_MAX_MESSAGES] = { "b64:000:Invalid Message Code.",
	                                    "b64:001:Syntax Error -- check help (-h) for usage.",
	                                    "b64:002:File Error Opening/Creating Files.",
	                                    "b64:003:File I/O Error -- Note: output file not removed.",
	                                    "b64:004:Error on output file close.",
	                                    "b64:005:linesize set to minimum.",
	                                    "b64:006:Syntax: Too many arguments." };

#define b64_message(ec) ((ec > 0 && ec < B64_MAX_MESSAGES) ? b64_msgs[ec] : b64_msgs[0])

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void encodeblock(unsigned char *in, unsigned char *out, int len)
{
	out[0] = (unsigned char)cb64[(int)(in[0] >> 2)];
	out[1] = (unsigned char)cb64[(int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4))];
	out[2] = (unsigned char)(len > 1 ? cb64[(int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6))] : '=');
	out[3] = (unsigned char)(len > 2 ? cb64[(int)(in[2] & 0x3f)] : '=');
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
__attribute__((used)) static int encode(FILE *infile, FILE *outfile, int linesize)
{
	unsigned char in[3];
	unsigned char out[4];
	int i, len, blocksout = 0;
	int retcode = 0;

	*in = (unsigned char)0;
	*out = (unsigned char)0;
	while (feof(infile) == 0) {
		len = 0;
		for (i = 0; i < 3; i++) {
			in[i] = (unsigned char)getc(infile);

			if (feof(infile) == 0) {
				len++;
			} else {
				in[i] = (unsigned char)0;
			}
		}
		if (len > 0) {
			encodeblock(in, out, len);
			for (i = 0; i < 4; i++) {
				if (putc((int)(out[i]), outfile) == EOF) {
					if (ferror(outfile) != 0) {
						perror(b64_message(B64_FILE_IO_ERROR));
						retcode = B64_FILE_IO_ERROR;
					}
					break;
				}
			}
			blocksout++;
		}
		if (blocksout >= (linesize / 4) || feof(infile) != 0) {
			if (blocksout > 0) {
				fprintf(outfile, "\n");
			}
			blocksout = 0;
		}
	}
	return (retcode);
}

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void decodeblock(unsigned char *in, unsigned char *out)
{
	out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
	out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
	out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
__attribute__((used)) static int decode(FILE *infile, FILE *outfile)
{
	int retcode = 0;
	unsigned char in[4];
	unsigned char out[3];
	int v;
	int i, len;

	*in = (unsigned char)0;
	*out = (unsigned char)0;
	while (feof(infile) == 0) {
		for (len = 0, i = 0; i < 4 && feof(infile) == 0; i++) {
			v = 0;
			while (feof(infile) == 0 && v == 0) {
				v = getc(infile);
				//printf("get %d\n", v);
				if (feof(infile) == 0) {
					v = ((v < 43 || v > 122) ? 0 : (int)cd64[v - 43]);
					if (v != 0) {
						v = ((v == (int)'$') ? 0 : v - 61);
					}
				}
			}

			if (feof(infile) == 0) {
				len++;
				if (v != 0) {
					in[i] = (unsigned char)(v - 1);
				}

			} else {
				in[i] = (unsigned char)0;
			}
		}
		//printf("len %d\n", len);
		if (len > 0) {
			decodeblock(in, out);
			for (i = 0; i < len - 1; i++) {
				// printf("put %d\n", (int) out[i]);
				if (putc((int)out[i], outfile) == EOF) {
					if (ferror(outfile) != 0) {
						perror(b64_message(B64_FILE_IO_ERROR));
						retcode = B64_FILE_IO_ERROR;
					}
					break;
				}
			}
		}
	}
	return (retcode);
}

int decodeChar(char *inchar, char *outchar, int len_input, int *len_output)
{
	int retcode = -1;
	unsigned char in[4];
	unsigned char out[3];
	int v;
	int i, len, total_len = 0;

	*in = (unsigned char)0;
	*out = (unsigned char)0;
	int k = 0, length_check = 0;
	while (k < len_input) {
		//while( feof( infile ) == 0 ) {

		for (len = 0, i = 0; i < 4 && k < len_input; i++) {
			v = 0;
			//printf("len_input: %d K: %d char is %c-", len_input, k, inchar[k]);
			v = inchar[k];
			v = ((v < 43 || v > 122) ? 0 : (int)cd64[v - 43]);
			if (v != 0) {
				v = ((v == (int)'$') ? 0 : v - 61);
			}
			if (v != 0) {
				in[i] = (unsigned char)(v - 1);
				len++;
				length_check++;
			} else {
				in[i] = (unsigned char)0;
			}
			k++;
			//printf("%x\n", in[i]);
		}
		char temp_decode[2];
		if (len > 0) {
			decodeblock(in, out);
			for (i = 0; i < len - 1; i++) {
				sprintf(temp_decode, "%02x", (int)out[i]);
				if ((i == len - 2 && len < 3) || (i == len - 2 && length_check == len_input)) {
					strcat(outchar, temp_decode);
					total_len += 2;
					goto end;
				} else {
					strcat(outchar, temp_decode);
					strcat(outchar, " ");
					//sprintf(outchar, "%s%s ", outchar, temp_decode);
					total_len += 3;
				}
			}
		}
	}
end:
	*len_output = total_len;
	retcode = 0;
	return retcode;
	//
}

/*
** b64
**
** 'engine' that opens streams and calls encode/decode
*/
#if 0
static int b64( char opt, char *infilename, char *outfilename, int linesize )
{
	FILE *infile;
	int retcode = B64_FILE_ERROR;

	if( !infilename ) {
		infile = stdin;
	}
	else {
		infile = fopen( infilename, "rb" );
	}
	if( !infile ) {
		perror( infilename );
	}
	else {
		FILE *outfile;
		if( !outfilename ) {
			outfile = stdout;
		}
		else {
			outfile = fopen( outfilename, "wb" );
		}
		if( !outfile ) {
			perror( outfilename );
		}
		else {
			if( opt == 'e' ) {
				retcode = encode( infile, outfile, linesize );
			}
			else {
				retcode = decode( infile, outfile );
			}
			if( retcode == 0 ) {
				if (ferror( infile ) != 0 || ferror( outfile ) != 0) {
					perror( b64_message( B64_FILE_IO_ERROR ) );
					retcode = B64_FILE_IO_ERROR;
				}
			}
			if( outfile != stdout ) {
				if( fclose( outfile ) != 0 ) {
					perror( b64_message( B64_ERROR_OUT_CLOSE ) );
					retcode = B64_FILE_IO_ERROR;
				}
			}
		}
		if( infile != stdin ) {
			if( fclose( infile ) != 0 ) {
				perror( b64_message( B64_ERROR_OUT_CLOSE ) );
				retcode = B64_FILE_IO_ERROR;
			}
		}
	}

	return( retcode );
}
#endif
/*
** showuse
**
** display usage information, help, version info
*/
/*
static void showuse( int morehelp )
{
	{
		printf( "\n" );
		printf( "  b64	  (Base64 Encode/Decode)	Bob Trower 2001/08/03 \n" );
		printf( "		   (C) Copr Bob Trower 1986-2015	 Version 0.94R \n" );
		printf( "  Usage:   b64 -option [-l<num>] [<FileIn> [<FileOut>]]\n" );
		printf( "  Purpose: This program is a simple utility that implements\n" );
		printf( "		   Base64 Content-Transfer-Encoding (RFC1113).\n" );
	}
	if( morehelp == 0 ) {
		printf( "		   Use -h option for additional help.\n" );
	}
	else {
		printf( "  Options: -e  encode to Base64   -h  This help text.\n" );
		printf( "		   -d  decode from Base64 -?  This help text.\n" );
		printf( "		   -t  Show test instructions. Under Windows the\n" );
		printf( "			   following command line will pipe the help\n" );
		printf( "			   text to run a test:\n" );
		printf( "				   b64 -t | cmd\n" );
		printf( "  Note:	-l  use to change line size (from 72 characters)\n" );
		printf( "  Returns: 0 = success.  Non-zero is an error code.\n" );
		printf( "  ErrCode: 1 = Bad Syntax, 2 = File Open, 3 = File I/O\n" );
		printf( "  Example: b64 -e binfile b64file	 <- Encode to b64\n" );
		printf( "		   b64 -d b64file binfile	 <- Decode from b64\n" );
		printf( "		   b64 -e -l40 infile outfile <- Line Length of 40\n" );
		printf( "  Note:	Will act as a filter, but this should only be\n" );
		printf( "		   used on text files due to translations made by\n" );
		printf( "		   operating systems.\n" );
		printf( "  Source:  Source code and latest releases can be found at:\n" );
		printf( "		   http://base64.sourceforge.net\n" );
		printf( "  Release: 0.94.00, Thu Oct 29 01:36:00 2015, ANSI-SOURCE C\n" );
	}
}

void dotest(void)
{
	printf(":See source code at sourceforge.net for\n");
	printf(":Unit test information.\n");
	printf(":\n");
	printf(":Under Windows the following batch file will\n");
	printf(":do a quick reality check:\n");
	printf("\n");
	printf("@echo off&cls\n");
	printf("copy /b /y b64.exe b64.bin 1>>nul 2>>nul\n");
	printf("b64 -e b64.bin b64.b64\n");
	printf("b64 -d b64.b64 b64.bin.exe\n");
	printf("fc /b b64.exe b64.bin.exe\n");
	printf(":/\\ Should say no differences enountered /\\\n");
	printf("\n");
}
*/
#define B64_DEF_LINE_SIZE 72
#define B64_MIN_LINE_SIZE 4

#define THIS_OPT(ac, av) ((char)(ac > 1 ? av[1][0] == '-' ? av[1][1] : 0 : 0))

/*
** main
**
** parse and validate arguments and call b64 engine or help
*/
/*
int main( int argc, char **argv )
{
	// Example
	char in[] = "AIDYAAAA";
	//char *in_ptr = &in[0];
	char out[256]="";
	//char *out_ptr = &out;
	size_t len = sizeof(in)-1;
	int outlen;
	decode_char(&in[0], &out[0], (int) len, &outlen);
	printf("ans is %s\n", &out[0]);


	char opt = (char) 0;
	int retcode = 0;
	int linesize = B64_DEF_LINE_SIZE;
	char *infilename = NULL, *outfilename = NULL;
	while( THIS_OPT( argc, argv ) != (char) 0 ) {
		switch( THIS_OPT(argc, argv) ) {
			case 'l':
					linesize = atoi( &(argv[1][2]) );
					if( linesize < B64_MIN_LINE_SIZE ) {
						linesize = B64_MIN_LINE_SIZE;
						printf( "%s\n", b64_message( B64_LINE_SIZE_TO_MIN ) );
					}
					break;
			case '?':
			case 'h':
					opt = 'h';
					break;
			case 'e':
			case 'd':
					opt = THIS_OPT(argc, argv);
					break;
			case 't':
					opt = THIS_OPT(argc, argv);
					break;
			default:
					opt = (char) 0;
					break;
		}
		argv++;
		argc--;
	}

	if( argc > 3 ) {
		printf( "%s\n", b64_message( B64_SYNTAX_TOOMANYARGS ) );
		opt = (char) 0;
	}
	switch( opt ) {
		case 'e':
		case 'd':
			infilename = argc > 1 ? argv[1] : NULL;
			outfilename = argc > 2 ? argv[2] : NULL;
			retcode = b64( opt, infilename, outfilename, linesize );
			break;
		case 0:
			if( argv[1] == NULL ) {
				showuse( 0 );
			}
			else {
				retcode = B64_SYNTAX_ERROR;
			}
			break;
		case 'h':
			showuse( (int) opt );
			break;
		case 't':
			dotest();
			break;

	}
	if( retcode != 0 ) {
		printf( "%s\n", b64_message( retcode ) );
	}

	return( retcode );

}
*/
