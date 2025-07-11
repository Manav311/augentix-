#ifndef HTTP_FLV_PARSER_H_
#define HTTP_FLV_PARSER_H_

#include "http_parser.h"
#include "stdbool.h"

#define MAX_HEADERS 13
#define MAX_HEADER_SIZE 512
#define MAX_BODY_SIZE (48 * 1024)
#define RET_BUF_SIZE (1024)

/***
 * Message: This is a structure containing not only request http message 
 *                 but response http message
 */
typedef struct message {
	enum http_parser_type type;
	enum http_method method;
	int status_code;

	char request_url[MAX_HEADER_SIZE];
	char response_status[MAX_HEADER_SIZE];
	/* body of request message */
	char body[MAX_BODY_SIZE];
	size_t body_size;

	/* this two will be used to response */
	//char ret_body[MAX_ELEMENT_SIZE];
	//char ret_head[MAX_ELEMENT_SIZE];

	/* content type of request message */
	enum { CONTENT_TYPE_JSON = 0, CONTENT_TYPE_UNKNOWN, CONTENT_TYPE_NUM } content_type;
	size_t content_length;

	const char *host;
	uint16_t port;
	int should_keep_alive;
	int num_headers;
	char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];

	/* here is a state machine for extracting header for http_parser */
	enum { NONE = 0, FIELD, VALUE } last_header_element;

	/* upgrade body */
	const char *upgrade;

	unsigned short http_minor;
	unsigned short http_major;
	char chn_num;
	bool is_audio; /*is_audio*/

	int message_begin_cb_called;
	int headers_complete_cb_called;
	int message_complete_cb_called;
	int status_cb_called;
	int message_complete_on_eof;
	int body_is_final;
} Message;

struct http_parser *parser_init(enum http_parser_type type);
size_t parse(struct http_parser *, const char *buf, size_t len, Message *m);
void freeHttpFlvParser(struct http_parser *);

#endif /* HTTP_FLV_PARSER_H_ */
