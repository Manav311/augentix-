#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "g72x.h"

/*
 * Display usage, examples, and available arguments.
 */
void usage(const char *basename)
{
	printf("G.726 software codec sample program\n");
	printf("Usage:   %s -a action -f format -r kbps -i in_file -o out_file\n", basename);
	printf("Example: %s -a enc -f alaw -r 40 -i sample3.alaw -o alaw_encode_40.g726\n", basename);
	printf("         %s -a dec -f alaw -r 40 -i alaw_encode_40.g726 -o g726_decode.alaw\n", basename);
	printf("\n");
	printf("Getting help: [-h|?]\n");
	printf("\n");
	printf("Options:\n");
	printf("-a action      set codec action:\n");
	printf("    enc            generate G.726 from PCM\n");
	printf("    dec            generate PCM from G.726\n");
	printf("-f format      set PCM format:\n");
	printf("    s16le          signed 16-bit little-endian\n");
	printf("    alaw           A-law\n");
	printf("    mulaw          mu-law\n");
	printf("-r kbps        set G.726 transmission rate (kbit/s):\n");
	printf("    16             G.726-16 (2-bit)\n");
	printf("    24             G.726-24 (3-bit)\n");
	printf("    32             G.726-32 (4-bit)\n");
	printf("    40             G.726-40 (5-bit)\n");
	printf("-i in_file     read file from in_file\n");
	printf("-o out_file    write file to out_file\n");
	printf("\n\n");
}

/*
 * Pack output codes into bytes and write them to a file.
 * Returns 1 if there is residual output, else returns 0.
 */
int pack(unsigned code, int bits, FILE *file)
{
	static unsigned int out_buffer = 0;
	static int out_bits = 0;
	unsigned char out_byte;

	out_buffer |= (code << out_bits);
	out_bits += bits;
	if (out_bits >= 8) {
		out_byte = out_buffer & 0xff;
		out_bits -= 8;
		out_buffer >>= 8;
		fwrite(&out_byte, sizeof(char), 1, file);
	}
	return (out_bits > 0);
}

/*
 * Unpack input codes and pass them back as bytes.
 * Returns 1 if there is residual input, returns -1 if eof, else returns 0.
 */
int unpack(unsigned char *code, int bits, FILE *file)
{
	static unsigned int in_buffer = 0;
	static int in_bits = 0;
	unsigned char in_byte;

	if (in_bits < bits) {
		if (fread(&in_byte, sizeof(char), 1, file) != 1) {
			*code = 0;
			return (-1);
		}
		in_buffer |= (in_byte << in_bits);
		in_bits += 8;
	}
	*code = in_buffer & ((1 << bits) - 1);
	in_buffer >>= bits;
	in_bits -= bits;
	return (in_bits > 0);
}

int main(int argc, char **argv)
{
	g726_state encoder_state;
	g726_state decoder_state;
	unsigned char sample_char;
	short sample_short;
	unsigned char code;
	int resid;
	int in_coding;
	int out_coding;
	int in_size;
	int out_size;
	unsigned *in_buf;
	int (*enc_routine)();
	int enc_bits;
	int (*dec_routine)();
	int dec_bits;
	FILE *in_file = NULL;
	FILE *out_file = NULL;

	extern char *optarg;
	int c;
	char *arg_action;
	char *arg_format;
	short arg_kbps = 0;
	char *arg_in_file = NULL;
	char *arg_out_file = NULL;

	/* Process program arguments */
	while ((c = getopt(argc, argv, "a:f:r:i:o:h")) != -1) {
		switch (c) {
		case 'a':
			arg_action = optarg;
			break;
		case 'f':
			arg_format = optarg;
			break;
		case 'r':
			arg_kbps = atoi(optarg);
			break;
		case 'i':
			arg_in_file = optarg;
			break;
		case 'o':
			arg_out_file = optarg;
			break;
		case 'h':
		case '?':
		default:
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	if (!strcmp(arg_action, "enc")) {
		/* Generate G.726 from PCM */
		/* Set input format */
		if (!strcmp(arg_format, "s16le")) {
			in_coding = AUDIO_ENCODING_LINEAR;
			in_size = sizeof(short);
			in_buf = (unsigned *)&sample_short;
		} else if (!strcmp(arg_format, "alaw")) {
			in_coding = AUDIO_ENCODING_ALAW;
			in_size = sizeof(char);
			in_buf = (unsigned *)&sample_char;
		} else if (!strcmp(arg_format, "mulaw")) {
			in_coding = AUDIO_ENCODING_ULAW;
			in_size = sizeof(char);
			in_buf = (unsigned *)&sample_char;
		} else {
			fprintf(stderr, "Invalid format '%s'.", arg_format);
			exit(EXIT_FAILURE);
		}

		/* Set G.726 encode rate */
		switch (arg_kbps) {
		case 16:
			enc_routine = g726_16_encoder;
			enc_bits = 2;
			break;
		case 24:
			enc_routine = g726_24_encoder;
			enc_bits = 3;
			break;
		case 32:
			enc_routine = g726_32_encoder;
			enc_bits = 4;
			break;
		case 40:
			enc_routine = g726_40_encoder;
			enc_bits = 5;
			break;
		default:
			fprintf(stderr, "Invalid G.726 encode rate (kbit/s) '%d'.", arg_kbps);
			exit(EXIT_FAILURE);
		}

		/* Prepare input and output files */
		if ((in_file = fopen(arg_in_file, "rb")) == NULL) {
			fprintf(stderr, "Failed to read file '%s'.", arg_in_file);
			exit(EXIT_FAILURE);
		}

		if ((out_file = fopen(arg_out_file, "wb")) == NULL) {
			fclose(in_file);
			fprintf(stderr, "Failed to write file '%s'.", arg_out_file);
			exit(EXIT_FAILURE);
		}

		/* Initialize encoder state */
		g726_init_state(&encoder_state);

		/* Read input file and process */
		while (fread(in_buf, in_size, 1, in_file) == 1) {
			code = (*enc_routine)(in_size == 2 ? sample_short : sample_char, in_coding, &encoder_state);
			resid = pack(code, enc_bits, out_file);
		}

		/* Write zero codes until all residual codes are written out */
		while (resid) {
			resid = pack(0, enc_bits, out_file);
		}
	} else if (!strcmp(arg_action, "dec")) {
		/* Generate PCM from G.726 */
		/* Set output format */
		if (!strcmp(arg_format, "s16le")) {
			out_coding = AUDIO_ENCODING_LINEAR;
			out_size = sizeof(short);
		} else if (!strcmp(arg_format, "alaw")) {
			out_coding = AUDIO_ENCODING_ALAW;
			out_size = sizeof(char);
		} else if (!strcmp(arg_format, "mulaw")) {
			out_coding = AUDIO_ENCODING_ULAW;
			out_size = sizeof(char);
		} else {
			fprintf(stderr, "Invalid format '%s'.", arg_format);
			exit(EXIT_FAILURE);
		}

		/* Set G.726 decode rate */
		switch (arg_kbps) {
		case 16:
			dec_routine = g726_16_decoder;
			dec_bits = 2;
			break;
		case 24:
			dec_routine = g726_24_decoder;
			dec_bits = 3;
			break;
		case 32:
			dec_routine = g726_32_decoder;
			dec_bits = 4;
			break;
		case 40:
			dec_routine = g726_40_decoder;
			dec_bits = 5;
			break;
		default:
			fprintf(stderr, "Invalid G.726 decode rate (kbit/s) '%d'.", arg_kbps);
			exit(EXIT_FAILURE);
		}

		/* Prepare input and output files */
		if ((in_file = fopen(arg_in_file, "rb")) == NULL) {
			fprintf(stderr, "Failed to read file '%s'.", arg_in_file);
			exit(EXIT_FAILURE);
		}

		if ((out_file = fopen(arg_out_file, "wb")) == NULL) {
			fclose(in_file);
			fprintf(stderr, "Failed to write file '%s'.", arg_out_file);
			exit(EXIT_FAILURE);
		}

		/* Initialize decoder state */
		g726_init_state(&decoder_state);

		/* Read and unpack input codes and process them */
		while (unpack(&code, dec_bits, in_file) >= 0) {
			sample_short = (*dec_routine)(code, out_coding, &decoder_state);
			if (out_size == 2) {
				fwrite(&sample_short, out_size, 1, out_file);
			} else {
				code = (unsigned char)sample_short;
				fwrite(&code, out_size, 1, out_file);
			}
		}
	}

	fclose(in_file);
	fclose(out_file);

	return EXIT_SUCCESS;
}
