#ifndef HTTP_FLV_H
#define HTTP_FLV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h> // for write

#include <mpi_enc.h>

#include "http_flv_parser.h"

void HTTP_checkResponseSize(const char *src, uint32_t strlen, char *size);
int HTTP_executeDeviceCmd(Message *m);
int HTTP_setlistenPort(int port, int timeout_s, char *ip_address);
bool HTTP_checkCodecInvalid(char chn_num);
int HTTP_checkCodecId(char chn_num, MPI_VENC_TYPE_E *type);

#endif
