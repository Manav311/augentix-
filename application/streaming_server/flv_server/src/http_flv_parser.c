#include "http_flv_parser.h"

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "json.h"

#include "log_define.h"

#define nullptr NULL

int currently_parsing_eof = 0;

size_t strnlen(const char *s, size_t maxlen)
{
	const char *p = (const char *)memchr(s, '\0', maxlen);
	if (p == NULL)
		return maxlen;

	return p - s;
}

size_t strlncat(char *dst, size_t len, const char *src, size_t n)
{
	size_t slen = strnlen(src, n);
	size_t dlen = strnlen(dst, len);

	if (dlen < len) {
		size_t rlen = len - dlen;
		size_t ncpy = slen < rlen ? slen : (rlen - 1);
		memcpy(dst + dlen, src, ncpy);
		dst[dlen + ncpy] = '\0';
	}

	assert(len > slen + dlen);
	return slen + dlen;
}

int message_begin_cb(struct http_parser *p)
{
	Message *m = (Message *)p->data;
	m->message_begin_cb_called = true;
	return 0;
}

int header_field_cb(struct http_parser *p, const char *buf, size_t len)
{
	Message *m = (Message *)p->data;

	if (m->last_header_element != FIELD)
		m->num_headers++;

	strlncat(m->headers[m->num_headers - 1][0], sizeof(m->headers[m->num_headers - 1][0]), buf, len);
	m->last_header_element = FIELD;
	return 0;
}

int header_value_cb(struct http_parser *p, const char *buf, size_t len)
{
	Message *m = (Message *)p->data;

	strlncat(m->headers[m->num_headers - 1][1], sizeof(m->headers[m->num_headers - 1][1]), buf, len);
	m->last_header_element = VALUE;
	return 0;
}

int request_url_cb(struct http_parser *p, const char *buf, size_t len)
{
	Message *m = (Message *)p->data;
	strlncat(m->request_url, sizeof(m->request_url), buf, len);
	flv_server_log_info("get request url: %s", m->request_url);

	return 0;
}

int response_status_cb(struct http_parser *p, const char *buf, size_t len)
{
	Message *m = (Message *)p->data;
	m->status_cb_called = true;

	strlncat(m->response_status, sizeof(m->response_status), buf, len);
	return 0;
}

void check_body_is_final(const struct http_parser *p)
{
	Message *m = (Message *)p->data;

	if (m->body_is_final) {
		flv_server_log_err("*** Error http_body_is_final() should return 1 "
		                   "on last on_body callback call "
		                   "but it doesn't! ***");
		assert(0);
		abort();
	}
	m->body_is_final = http_body_is_final(p);
}

int body_cb(struct http_parser *p, const char *buf, size_t len)
{
	Message *m = (Message *)p->data;

	strlncat(m->body, sizeof(m->body), buf, len);
	m->body_size += len;
	check_body_is_final(p);
	return 0;
}

int headers_complete_cb(struct http_parser *p)
{
	Message *m = (Message *)p->data;

	/* get general information */
	m->method = (enum http_method)p->method;
	m->status_code = p->status_code;
	m->http_major = p->http_major;
	m->http_minor = p->http_minor;
	m->headers_complete_cb_called = true;
	m->should_keep_alive = http_should_keep_alive(p);

	/* parse header to value */
	for (int i = 0; i < m->num_headers; ++i) {
		if (strcmp(m->headers[i][0], "Host") == 0) {
			m->host = m->headers[i][1];
		} else if (strcmp(m->headers[i][0], "Content-Type") == 0) {
			if (strcmp(m->headers[i][1], "application/json") == 0) {
				m->content_type = CONTENT_TYPE_JSON;
			} else {
				m->content_type = CONTENT_TYPE_UNKNOWN;
			}
		} else if (strcmp(m->headers[i][0], "Content-Length") == 0) {
			m->content_length = (size_t)atoi(m->headers[i][1]);
		} else if (strcmp(m->headers[i][0], "User-Agent") == 0) {
			/* do nothing */
			flv_server_log_debug("User-agent: %s", m->headers[i][1]);
		}
	}

	return 0;
}

int message_complete_cb(struct http_parser *p)
{
	Message *m = (Message *)p->data;

	if (m->should_keep_alive != http_should_keep_alive(p)) {
		flv_server_log_err("*** Error http_should_keep_alive() should have same "
		                   "value in both on_message_complete and on_headers_complete "
		                   "but it doesn't! ***");
		assert(0);
		abort();
	}

	if (m->body_size && http_body_is_final(p) && !m->body_is_final) {
		flv_server_log_err("*** Error http_body_is_final() should return 1 "
		                   "on last on_body callback call "
		                   "but it doesn't! ***");
		assert(0);
		abort();
	}

	/* increase message counter */
	m->message_complete_cb_called = true;
	m->message_complete_on_eof = currently_parsing_eof;

	return 0;
}

http_parser_settings settings = { message_begin_cb, request_url_cb,      response_status_cb,
	                          header_field_cb,  header_value_cb,     headers_complete_cb,
	                          body_cb,          message_complete_cb, nullptr,
	                          nullptr };

struct http_parser *parser_init(enum http_parser_type type)
{
	struct http_parser *parser = NULL;
	flv_server_log_info("enter in");

	parser = (struct http_parser *)malloc(sizeof(struct http_parser));

	if (!parser) {
		flv_server_log_err("failed mallor parser: %s", strerror(errno));
		return parser;
	}

	http_parser_init(parser, type);
	return parser;
}

void freeHttpFlvParser(struct http_parser *parser)
{
	free(parser);
}

size_t parse(http_parser *parser, const char *buf, size_t len, Message *m)
{
	size_t nparsed;
	parser->data = (void *)m;
	currently_parsing_eof = (len == 0);

	/* start parsing */
	nparsed = http_parser_execute(parser, &settings, buf, len);

	if (nparsed != len) {
		flv_server_log_err("*** %s ***", http_errno_description(HTTP_PARSER_ERRNO(parser)));
		return -1;
	}

	return nparsed;
}
