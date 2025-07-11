#include "https_flv_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "log_define.h"
#include "http_parser.h"
#include "http_flv_parser.h"

#include "http_flv_server.h"
#include "http_flv.h"
#include "https_flv.h"

#include "video.h"
#include "audio.h"

static const char *g_device[] = { "default", "hw:0,0" }; /* sound device */
extern int g_run_flag;

int writeTLSFLVOutput(void *src, uint32_t len, int clientfd)
{
	int ret = SSL_write((SSL *)clientfd, src, len);
	if (ret != (int)len) {
		flv_server_log_warn("ret %d != %d", ret, len);
	}
	return ret;
}

static int sendResponse(char *response, int len, int socketfd)
{
	flv_server_log_info("send size:%d", len);

	char size;
	HTTP_checkResponseSize(&response[0], len, &size);

	flv_server_log_debug("==Response Message[%d]== %s", size, response);

	return SSL_write((SSL *)socketfd, response, size);
}

void *__processTLSMessage(void *argv)
{
	SSL *ssl = (SSL *)argv;
	int client_socket = SSL_get_fd(ssl);

	Message *m;
	char *buf;
	int ret = 0;
	if (SSL_accept(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
	}

	buf = malloc(RET_BUF_SIZE);
	if (!buf) {
		flv_server_log_err("Failed to allocate memory !");
		goto close_fd;
	}

	/* initialize parser */
	m = malloc(sizeof(Message));
	if (!m) {
		flv_server_log_err("Failed to allocate memory !");
		goto free_buf;
	}

	struct http_parser *parser = parser_init(HTTP_REQUEST);
	if (!parser) {
		flv_server_log_err("Failed to allocate memory !");
		goto free_msg;
	}

	do {
		int len, nparsed;
		bzero(m, sizeof(*m));

		/* receive a complete message */
		do {
			len = SSL_read(ssl, buf, RET_BUF_SIZE);
			flv_server_log_info("recv len:%d.", len);
			if (len <= 0) {
				goto end;
			}

			/* print recevied string */
			buf[len] = '\0';
			flv_server_log_debug("==Received String== %s.", buf);

			/* start parsing */
			nparsed = parse(parser, buf, len, m);

			if (nparsed == -1) {
				goto end;
			}
		} while ((!m->body_is_final && m->content_length) && g_run_flag);

		/* response*/
		ret = HTTP_executeDeviceCmd(m);

		char dateBuf[32];
		time_t now = time(0);
		struct tm tm = *gmtime(&now);
		strftime(dateBuf, sizeof(dateBuf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
		char response[256] = { 0 };
		int nsent = -1;

		if (ret != 0) {
			if (ret == -EACCES) {
				snprintf(&response[0], sizeof(response),
				         "HTTP/1.1 500 Internal Server Error\r\nServer: %s\r\nDate: %s\r\n\r\n",
				         "Augentix", dateBuf);
			} else {
				snprintf(&response[0], sizeof(response),
				         "HTTP/1.1 400 Bad Request\r\nServer: %s\r\nDate: %s\r\n\r\n", "Augentix",
				         dateBuf);
			}

			nsent = sendResponse(&response[0], strlen(response), (int)ssl);
			goto end;
		}

		/*init bistream*/
		int ret;
		uint8_t aac_ret;

		MPI_BCHN bchn;
		MPI_VENC_TYPE_E type;
		VideoStreamData stream_data;
		bool is_first_frame = true;
		ret = initStream(m->chn_num, &bchn); /*bchn -> bchn*/
		if (ret != 0) {
			flv_server_log_err("failed to init bitstream");
			snprintf(&response[0], sizeof(response),
			         "HTTP/1.1 500 Internal Server Error\r\nServer: %s\r\nDate: %s\r\n\r\n", "Augentix",
			         dateBuf);
			sendResponse(&response[0], strlen(response), (int)ssl);
			uninitStream(&bchn);
			goto end;
		}

		/*audio init*/
		snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
		snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
		unsigned int rate = 8000;
		int dev_id = 0;
		snd_pcm_uframes_t frame = 1024;
		int channel = 1;
		int buf_len = frame * 2 * 1;
		char aac_buf[buf_len];
		char pcm_buf[buf_len];
		TyMediaAACHandle hdl;
		int frames = 1024;
		bool is_first_aac = true; /*is_first_aac*/
		uint32_t audio_ts = 0;
		snd_pcm_t *p_capture;

		bool is_audio = m->is_audio; /*is_audio*/

		if (is_audio) {
			ret = agtxPcmInit(&p_capture, g_device[dev_id], stream, format, frame, rate, channel);
			if (ret < 0) {
				flv_server_log_err("failed to init pcm, %d.", ret);
				flv_server_log_err("failed to init audio, send video only.");
				is_audio = false;
			} else {
				ret = AAC_encoderInit(&hdl, channel, rate, rate);
				if (ret < 0) {
					flv_server_log_err("Failed to init AAC encoder.");
					is_audio = false;
					AAC_encoderUninit(&hdl);
				} else {
					snd_pcm_nonblock(p_capture, 1);
				}
			}
		}

		snprintf(&response[0], sizeof(response),
		         "HTTP/1.1 200 OK\r\n"
		         "Server: %s Server <0.1>\r\n"
		         "Content-Type:  video/x-flv\r\n"
		         "Connection: keep-alive\r\n"
		         "Expires: -1\r\n"
		         "Access-Control-Allow-Origin: *\r\n"
		         "Access-Control-Allow-Credentials: true\r\n\r\n",
		         "Augentix");
		nsent = sendResponse(&response[0], strlen(response), (int)ssl);

		MediaSrcInfo src_info;
		src_info.chn_num = m->chn_num;
		src_info.output_fd = (int)ssl;
		src_info.fCheckCodecChange = HTTP_checkCodecInvalid;
		src_info.fWriteFlv = writeTLSFLVOutput;

		FLV_writeFLVHeader(&src_info, is_audio, true);

		unsigned int time_diff = 0;
		unsigned int start_time_jiff = 0;
		uint8_t vps_offset = 0;
		uint8_t sps_offset = 0;
		uint8_t pps_offset = 0;
		uint32_t size = 0;
		uint8_t *vps = NULL;
		uint8_t *sps = NULL;
		uint8_t *pps = NULL;
		uint8_t *idr = NULL;

		const uint8_t k_nalu_header_offset = 4;
		const uint8_t k_aac_es_offset = 7;

		while (((g_run_flag) && (nsent != -1)) /*&& m->should_keep_alive, this is ffmpeg bug*/) {
			ret = readFrame(&bchn, &stream_data);
			if (ret != 0) {
				flv_server_log_err("no data");
				/*getbistream error handling*/
				snprintf(&response[0], sizeof(response),
				         "HTTP/1.1 500 Internal Server Error\r\nServer: %s\r\nDate: %s\r\n\r\n",
				         "Augentix", dateBuf);
				sendResponse(&response[0], strlen(response), (int)ssl);
				nsent = -1;
				break;
			}

			/*need to wait until first Idr*/
			if ((is_first_frame) && (stream_data.params.seg[0].type != MPI_FRAME_TYPE_SPS)) {
				flv_server_log_info("Wait for first I frame.");
				releaseFrame(&bchn, &stream_data);
				continue;
			}

			if (false == HTTP_checkCodecInvalid(m->chn_num)) {
				/*change codec, getbistream error handling*/
				snprintf(&response[0], sizeof(response),
				         "HTTP/1.1 500 Internal Server Error\r\nServer: %s\r\nDate: %s\r\n\r\n",
				         "Augentix", dateBuf);
				sendResponse(&response[0], strlen(response), (int)ssl);
				flv_server_log_err("no data");
				nsent = -1;
				break;
			}

			if (HTTP_checkCodecId(m->chn_num, &type) != 0) {
				flv_server_log_err("failed to get codec");
			}

			flv_server_log_info("get codec: %d", type);
			if (src_info.video_codec != type && is_first_frame == false) {
				flv_server_log_err("change codec, need err handling !");
			}
			src_info.video_codec = type;

			if (0 > checkFrameNalus(&src_info, &stream_data)) {
				goto release_frame;
			}

			/*Mux video tag + send*/
			if (is_first_frame) {
				start_time_jiff = timespec_to_ms(&stream_data.params.timestamp);
				time_diff = 0;
				audio_ts = 0; /*for avsync*/
				if (src_info.video_codec == MPI_VENC_TYPE_H264) {
					nsent = FLV_writeAVCSeqHeaderTag(
					        &src_info, NULL, 0,
					        stream_data.params.seg[0].uaddr + k_nalu_header_offset,
					        stream_data.params.seg[0].size - k_nalu_header_offset,
					        stream_data.params.seg[1].uaddr + k_nalu_header_offset,
					        stream_data.params.seg[1].size - k_nalu_header_offset, time_diff);
				} else if (src_info.video_codec == MPI_VENC_TYPE_H265) {
					vps_offset = 24;
					sps_offset = 32;

					if (stream_data.params.seg[0].size != 75) {
						sps_offset = stream_data.params.seg[0].size -
						             (k_nalu_header_offset + vps_offset + k_nalu_header_offset +
						              k_nalu_header_offset + 7);
					}

					pps_offset = k_nalu_header_offset + vps_offset + k_nalu_header_offset +
					             sps_offset + k_nalu_header_offset;

					nsent = FLV_writeAVCSeqHeaderTag(
					        &src_info, stream_data.params.seg[0].uaddr + k_nalu_header_offset,
					        vps_offset,
					        stream_data.params.seg[0].uaddr + k_nalu_header_offset + vps_offset +
					                k_nalu_header_offset,
					        sps_offset, stream_data.params.seg[0].uaddr + pps_offset, 7, time_diff);
				}

				if (nsent < 0) {
					releaseFrame(&bchn, &stream_data);
					break;
				}

				is_first_frame = false;
			} else {
				time_diff = (timespec_to_ms(&stream_data.params.timestamp) - start_time_jiff);
				if (stream_data.params.frame_id == 0) {
					flv_server_log_notice("re-start enc.");
					audio_ts = time_diff;
				}
			}

			if (stream_data.params.seg[0].type == MPI_FRAME_TYPE_I) {
				if (src_info.video_codec == MPI_VENC_TYPE_H264) {
					flv_server_log_notice("send Idr without SPS, PPS.");
				} else if (src_info.video_codec == MPI_VENC_TYPE_H265) {
					flv_server_log_notice("send Idr without SPS, PPS.");
				}
			}

			if (stream_data.params.seg[0].type == MPI_FRAME_TYPE_SPS) {
				if (src_info.video_codec == MPI_VENC_TYPE_H264) {
					vps_offset = 0;
					sps_offset = stream_data.params.seg[0].size - k_nalu_header_offset;
					pps_offset = stream_data.params.seg[1].size - k_nalu_header_offset;
					size = stream_data.params.seg[2].size - k_nalu_header_offset;
					if (stream_data.params.seg_cnt > 2) {
						for (int i = 3; (unsigned)i < stream_data.params.seg_cnt; i++) {
							size += stream_data.params.seg[i].size;
						}
					}
					vps = NULL;
					sps = stream_data.params.seg[0].uaddr + k_nalu_header_offset;
					pps = stream_data.params.seg[1].uaddr + k_nalu_header_offset;
					idr = stream_data.params.seg[2].uaddr + k_nalu_header_offset;

					flv_server_log_debug("[%d]chn %d jiffies: %lu.", clientfd, src_info.chn_num,
					                     (unsigned long)stream_data.params.jiffies);
				} else if (src_info.video_codec == MPI_VENC_TYPE_H265) {
					vps_offset = 24;
					sps_offset = stream_data.params.seg[0].size -
					             (k_nalu_header_offset + vps_offset + k_nalu_header_offset +
					              k_nalu_header_offset + 7);
					pps_offset = 7;
					size = stream_data.params.seg[1].size - k_nalu_header_offset;

					vps = stream_data.params.seg[0].uaddr + k_nalu_header_offset;
					sps = stream_data.params.seg[0].uaddr + k_nalu_header_offset + vps_offset +
					      k_nalu_header_offset;
					pps = stream_data.params.seg[0].uaddr + k_nalu_header_offset + vps_offset +
					      k_nalu_header_offset + sps_offset + k_nalu_header_offset;
					idr = stream_data.params.seg[1].uaddr + k_nalu_header_offset;
					if (stream_data.params.seg_cnt > 2) {
						for (int i = 1; (unsigned)i < stream_data.params.seg_cnt; i++) {
							size += stream_data.params.seg[i].size;
						}
					}
				}
				flv_server_log_debug("[%d]chn %d jiffies: %lu", (int)ssl, src_info.chn_num,
				                     (unsigned long)stream_data.params.jiffies);

				nsent = FLV_writeAVCDataTag(&src_info, vps, vps_offset, sps, sps_offset, pps,
				                            pps_offset, idr, size, time_diff, 1);
				if (nsent == -EACCES) {
					releaseFrame(&bchn, &stream_data);
					snprintf(&response[0], sizeof(response),
					         "HTTP/1.1 500 Internal Server Error\r\nServer: %s\r\nDate: %s\r\n\r\n",
					         "Augentix", dateBuf);
					sendResponse(&response[0], strlen(response), (int)ssl);
					break;
				} else if (nsent == -EIO) { /*change profile*/

					audio_ts = time_diff;

					// nsent = FLV_writeAVCSeqHeaderTag(
					//         &src_info, NULL, 0,
					//         stream_data.params.seg[0].uaddr + k_nalu_header_offset,
					//         stream_data.params.seg[0].size - k_nalu_header_offset,
					//         stream_data.params.seg[1].uaddr + k_nalu_header_offset,
					//         stream_data.params.seg[1].size - k_nalu_header_offset, time_diff);
					nsent = FLV_writeAVCDataTag(
					        &src_info, NULL, 0,
					        stream_data.params.seg[0].uaddr + k_nalu_header_offset, sps_offset,
					        stream_data.params.seg[1].uaddr + k_nalu_header_offset, pps_offset,
					        stream_data.params.seg[2].uaddr + k_nalu_header_offset, size, time_diff,
					        1);

				} else if (nsent < 0) {
					releaseFrame(&bchn, &stream_data);
					break;
				}

			} else if (stream_data.params.seg[0].type == MPI_FRAME_TYPE_P) {
				/* MPI_VENC_TYPE_H264 & MPI_VENC_TYPE_H265 the same */
				size = stream_data.params.seg[0].size - k_nalu_header_offset;
				if (stream_data.params.seg_cnt > 1) {
					for (int i = 1; (unsigned)i < stream_data.params.seg_cnt; i++) {
						size += stream_data.params.seg[i].size;
					}
				}
				nsent = FLV_writeAVCDataTag(&src_info, NULL, 0, NULL, 0, NULL, 0,
				                            stream_data.params.seg[0].uaddr + k_nalu_header_offset,
				                            size, time_diff, 0);
				if (nsent < 0) {
					releaseFrame(&bchn, &stream_data);
					break;
				}
			}
release_frame:
			releaseFrame(&bchn, &stream_data);

			if (is_audio) {
				/*always flush pcm buffer to empty*/
				while ((g_run_flag == 1) && (ret != -EAGAIN)) {
					ret = snd_pcm_readi(p_capture, pcm_buf, frames);
					if (ret == -EPIPE) {
						snd_pcm_prepare(p_capture);
						flv_server_log_err("-EPIPE\n");
						continue;
					} else if (ret == -EAGAIN) {
						/* means there is no data, break for loop */
						break;
					} else if (ret < 0) {
						flv_server_log_err("snd pcm unknown err: %d", ret);
						break;
					}
					flv_server_log_info("get audio sample: %d", ret);
					int aac_recv = ret * 2;
					aac_ret = AAC_encoderGetData(&hdl, pcm_buf, ret * 2, ret, aac_buf, &aac_recv);
					if (aac_ret != AACENC_OK) {
						flv_server_log_err("aac_ret == %d", aac_ret);
						break;
					}

					audio_ts += 128;
					if (is_first_aac == true) {
						nsent = FLV_writeAACSeqHeaderTag(&src_info, rate, channel, 0);
						if (nsent < 0) {
							break;
						}
						is_first_aac = false;
					}

					nsent = FLV_writeAACDataTag(&src_info, (uint8_t *)&aac_buf[k_aac_es_offset],
					                            aac_recv - k_aac_es_offset, audio_ts);
					if (nsent < 0) {
						break;
					}
				}
			}
		}

		if (is_audio) {
			ret = agtxPcmUninit(p_capture);
			if (ret < 0) {
				flv_server_log_err("failed to uninit pcm, %d", ret);
			}
			ret = AAC_encoderUninit(&hdl);
			if (ret < 0) {
				flv_server_log_err("failed to uninit pcm, %d", ret);
			}
		}

		uninitStream(&bchn);

		if (nsent == -1) {
			flv_server_log_err("nsent == -1");
			goto end;
		}

	} while ((m->should_keep_alive) && g_run_flag);

	flv_server_log_debug("leave");
	freeHttpFlvParser(parser);
	free(m);
	free(buf);
	SSL_shutdown(ssl);
	SSL_free(ssl);

	close(client_socket);

	pthread_detach(pthread_self());
	return NULL;

end:
	flv_server_log_debug("parser free");
	freeHttpFlvParser(parser);
free_msg:
	free(m);
free_buf:
	free(buf);
close_fd:
	close(client_socket);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	pthread_detach(pthread_self());

	return NULL;
}

void *runHttpsServerListenThread(void *argv)
{
	HttpsServerInfo *info = (HttpsServerInfo *)argv;
	flv_server_log_info("Assigned port %d, src:%s.cert path:%s.key path:%s.", info->port, info->src_file,
	                    info->cert_file, info->key_file);

	int ret = 0;

	int server_sock, client_sock;
	static struct sockaddr_in cli_addr;
	socklen_t length = sizeof(cli_addr);
	int port = info->port;

	TLS_initOpenssl();
	info->ctx = TLS_createContext();
	if (info->ctx == NULL) {
		flv_server_log_err("failed to create tls context");

		return NULL;
	}
	ret = TLS_configureContext(info->ctx, &info->cert_file[0], &info->key_file[0]);
	if (ret != 0) {
		flv_server_log_err("failed to configure context: %d", ret);
		goto exit_ssl;
	}
	server_sock = HTTP_setlistenPort(port, 2, info->ip_address);

	while (g_run_flag) {
		client_sock = accept(server_sock, (struct sockaddr *)&cli_addr, &length);
		if (client_sock == -1) {
			flv_server_log_err("accept error\r\n");
		}

		flv_server_log_notice("Accept a connection from %s:%d.", "localhost", port);

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		info->client_socket = client_sock;

		info->ssl = SSL_new(info->ctx);
		if (info->ssl == NULL) {
			flv_server_log_err("Failed new ssl");
			continue;
		}
		if (1 != SSL_set_fd(info->ssl, client_sock)) {
			flv_server_log_err("failed to set fd.");
		}

		pthread_t t0;
		ret = pthread_create(&t0, &attr, __processTLSMessage, (void *)info->ssl);
		if (ret != 0) {
			flv_server_log_err("pthread_create __processTLSMessage failed, ret: %d", ret);
		}

		pthread_attr_destroy(&attr);
	}

exit_ssl:
	SSL_CTX_free(info->ctx);
	TLS_cleanupOpenssl();

	pthread_detach(pthread_self());
	return NULL;
}
