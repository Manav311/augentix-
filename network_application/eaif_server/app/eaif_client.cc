#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define CLI_USE_UNIX

#ifdef CLI_USE_UNIX
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else // CLI_USE_BOOST
#include "boost/asio.hpp"
using namespace boost;
#endif // !CLI_USE_UNIX

#include "eaif_trc.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

#define IMG_FMT_RGB "rgb"
#define IMG_FMT_JPEG "jpg"
#define IMG_FMT_MPI_JPEG "mpi_jpg"
#define IMG_FMT_MPI_RGB "mpi_rgb"
#define IMG_FMT_MPI_Y "mpi_y__"

struct timespec start;

#define RECV_BUF_SIZE (4096)
#define SEND_BUF_SIZE(length) ((((length)/4096) + 5) * 4096)

#ifndef CLI_USE_UNIX
class boost_service {
	public:
	boost_service():c(is){};
	int uconnect(const string& host, int port)
	{
		c.connect(asio::ip::tcp::endpoint(
	    asio::ip::address::from_string(host), port));
		return 0;
	}
	int post(const char* sendmsg, int size, char* buf, int rsize)
	{
		size_t recved = 0;
		try {
			c.send(asio::const_buffer(sendmsg, size));
			recved = c.receive(asio::buffer(buf, rsize));
			cout << "RECV from server:\n" << string(buf) << "\n";
		} catch (int e) {
			cout << "ERROR code is " << e << "\n";
			return -1;
		}
		return recved;
	}

	private:
	asio::io_service is;
	asio::ip::tcp::socket c;
};

std::string make_string(boost::asio::streambuf& streambuf)
{
  return {boost::asio::buffers_begin(streambuf.data()), 
          boost::asio::buffers_end(streambuf.data())};
}

#endif

class unix_service {
	public:
	~unix_service()
	{
		if (sockfd != -1)
			uclose();
	};
	int uconnect(const string& host, int port)
	{
		portno = port;
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("%d: Create socket fails error: %s(errno: %d)\n", __LINE__, strerror(errno), errno);
			return -1;
		}
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port   = htons(port);
		servaddr.sin_addr.s_addr = inet_addr(host.c_str());

		if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
			printf("%d: Connect to host %s sockfd:%d fails: %s(errno:%d)\n", __LINE__, host.c_str(), sockfd, strerror(errno), errno);
			uclose();
			return -1;
		}
		return 0;
	}

	int post(const char* sendmsg, int size, char* recvmsg, int rsize)
	{
		int ret = send(sockfd, sendmsg, size, 0);
		if (ret == -1) {
			printf("%d: Send message fails: %s(errno: %d)\n", __LINE__, strerror(errno), errno);
			uclose();
			return -1;
		}

		ret = recv(sockfd, recvmsg, rsize, 0);
		if (ret == -1) {
			printf("%d: Recv return message fails: %s(errno: %d)\n", __LINE__, strerror(errno), errno);
			uclose();
			return -1;
		}
		cout << "RECV from server:\n" << string(recvmsg) << "\n";
		return ret;
	}

	private:
	void uclose() { close(sockfd); sockfd = -1; };
	int sockfd = -1;
	int portno = -1;
	struct sockaddr_in servaddr;
};

namespace http_cli {
string BOUNDARY =  "---im147-takchoi.yu@augentix.com-9752478";
} // http_client

template<typename client_service>
class http_client{
	public:
	http_client(){};
	~http_client(){};
	int connect(std::string &host, int port)
	{
		return cli.uconnect(host, port);
	}
	int post(const char *data, int size)
	{
		return cli.post(data, size, buf, RECV_BUF_SIZE);
	}

	char buf[RECV_BUF_SIZE];
	string m_host;
	int m_port;
	client_service cli;
};

struct multipart_request {
	int content_length;
	int port;
	const char* host;
	const char* query_str;
	vector<pair<string, vector<uint8_t> > > contents;
	void fill_content(const char *name, const uint8_t *payload, int size);
	void to_str(string& ostr);
	char* to_str(int &size);
};

void multipart_request::fill_content(const char *name, const uint8_t *payload, int size)
{
	vector<uint8_t> content(size);
	memcpy(content.data(), payload, size);

	contents.push_back(
		pair<string, vector<uint8_t> >(string(name),content));
}

char* multipart_request::to_str(int& size)
{
#define NEWLINE_LENGTH (2)
#define PREFIX_LENGTH (2)

	content_length = 0;
	static string disposition = "Content-Disposition: form-data\"\"; name=\"\"; filename=\r\n\r\n";

	for (auto con : contents) {
		content_length +=
		PREFIX_LENGTH + http_cli::BOUNDARY.size() + NEWLINE_LENGTH +
		disposition.size() + 
		con.first.size() * 2 +
		con.second.size() + NEWLINE_LENGTH;
	}
	content_length += PREFIX_LENGTH + http_cli::BOUNDARY.size() + PREFIX_LENGTH + NEWLINE_LENGTH;

	char *msg_str = new char[SEND_BUF_SIZE(content_length)];
	size = sprintf(msg_str,
		"POST %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"User-Agent: EaifClientApp\r\n"
		"Accept: */*\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Connection: keep-alive\r\n"
      	"Content-Length: %d\r\n"
      	"Content-Type: multipart/form-data; boundary=%s\r\n\r\n",
      	query_str,
      	host, port,
      	content_length,
      	http_cli::BOUNDARY.c_str());

	for (auto con : contents) {
      	size += sprintf(&msg_str[size],
      		"--%s\r\n"
      		"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n\r\n",
      		http_cli::BOUNDARY.c_str(),
      		con.first.c_str(), con.first.c_str());
      	memcpy(&msg_str[size], con.second.data(), con.second.size());
      	size += con.second.size();
      	size += sprintf(&msg_str[size], "\r\n");
    }
    size += sprintf(&msg_str[size],
      		"--%s--\r\n", http_cli::BOUNDARY.c_str());
    return msg_str;
}


#define min(x,y) ((x) < (y))? (x) : (y)

inline const string help_str(void)
{
	return string("./eaif_client usage: <config.ini>\n\n"
	"\t /* config.ini format*/\n"
	"\t host= 127.0.0.1\n"
	"\t port= 40089\n"
	"\t query_api= /predict/yolov5\n"
	"\t img_file= ../data/obama.jpg\n"
	"\t img_fmt= jpg\n"
	"\t meta= {json object list} // default none\n"
	"\t snapshot_width= 1280\n"
	"\t snapshot_height= 720\n"
	"\n"
	"\t example meta format :\n"
	"\t {\"od\":[{\"obj\":{\"id\":0,\"rect\":[0,0,width,height],\"cat\":\"\",\"shaking\":0}}]}\n"
	"\n"
	"\t snapshot dimension only required for mpi_y__ and mpi_rgb\n");
}

inline const string help_str2(void)
{
	return string("./eaif_client usage: <host> <port> <query api> <img_file> <img_fmt> <meta_str>\n\n"
	"\t /* args example */\n"
	"\t host= 127.0.0.1\n"
	"\t port= 40089\n"
	"\t query_api= /predict/yolov5\n"
	"\t img_file= ../data/obama.jpg\n"
	"\t img_fmt= jpg\n"
	"\t meta= {json object list} // default none\n"
	"\t \n"
	"\t example meta format :\n"
	"\t {\"od\":[{\"obj\":{\"id\":0,\"rect\":[0,0,width,height],\"cat\":\"\",\"shaking\":0}}]}\n");
}

#ifdef _WIN32
#define DELIMITER "\r\n"
#else
#define DELIMITER "\n"
#endif

int main(int argc, char **argv)
{

	char host[256] = { };
	char query_api[256] = { };
	char img_fmt[16] = { };
	char time_spec[32] = { };
	char img_file[512] = { };
	uint8_t *img_data = nullptr;
	uint8_t *img_rgb = nullptr;
	char meta[1024] = { };
	int port = 0;
	int shape[3] = { };
	int fsize = 0;
	int ret = 1;
	char* sendmsg = nullptr;
	int snapshot_width = 0, snapshot_height = 0;

	if (!(argc == 2 || argc == 7)) {
		if (argc < 2) {
			cerr << "Wrong config format !" << "\n";
			cerr << help_str();
		} else if (argc > 2) {
			cerr << "argc is " << argc << "\n";
			cerr << help_str2();
		}
		return -1;
	}

	if (argc == 2) {
		FILE *fp = fopen(argv[1], "r");
		if (!fp) {
			cerr << "Cannot find " << string(argv[1]) << "\n";
			return 0;
		}

		if (fscanf(fp, "host= %s" DELIMITER, host) != 1 ||
			fscanf(fp, "port= %d" DELIMITER, &port) != 1 ||
			fscanf(fp, "query_api= %s" DELIMITER, query_api) != 1 ||
			fscanf(fp, "img_file= %s" DELIMITER, img_file) != 1 ||
			fscanf(fp, "img_fmt= %s" DELIMITER, img_fmt) != 1 ||
			fscanf(fp, "meta= %s" DELIMITER, meta) != 1) {
			cerr << "Wrong config format !" << "\n";
			cerr << help_str();
			return 0;
		}
		if (!strcmp(img_fmt, IMG_FMT_MPI_Y) ||
			!strcmp(img_fmt, IMG_FMT_MPI_RGB)) {
			if (fscanf(fp, "snapshot_width= %d" DELIMITER, &snapshot_width) != 1 ||
			   fscanf(fp, "snapshot_height= %d" DELIMITER, &snapshot_height) != 1) {
			   	cerr << "Please provide snapshot dimension" << "\n";
			    cerr << help_str();
			   }
		}
		fclose(fp);
	} else if (argc == 7) {
		strcpy(host, argv[1]);
		port = atoi(argv[2]);
		strcpy(query_api, argv[3]);
		strcpy(img_file, argv[4]);
		strcpy(img_fmt, argv[5]);
		strcpy(meta, argv[6]);
	}
	int is_img_fmt_mpi_img = 0;

	if ((strcmp(img_fmt, IMG_FMT_MPI_JPEG) != 0) &&
		(strcmp(img_fmt, IMG_FMT_MPI_Y) != 0)) {

		FILE *fp = fopen(img_file, "rb");
		if (!fp) {
			cerr << "Cannot find " << string(img_file) << "\n";
			return 0;
		}

		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		img_rgb = new uint8_t[fsize];
		rewind(fp);
		int readsize = fread(img_rgb, 1, fsize, fp);
		fclose(fp);
	    if (readsize != fsize) {
	        cerr << "Unexpected error during reading file : " << img_file << "\n";
	        delete[] img_rgb;
	        return -1;
	    }

	    /* stbi_load_from_memory   ( *buffer, len   , int *x, int *y, int *channels_in_file, int desired_channels); */
		img_data = stbi_load_from_memory(img_rgb, fsize, &shape[0], &shape[1], &shape[2], 0);

		if (!strcmp(img_fmt, IMG_FMT_RGB)) {
			delete[] img_rgb;
			img_rgb = nullptr;
			fsize = shape[0] * shape[1] * shape[2];
		} else if (!strcmp(img_fmt, IMG_FMT_JPEG)) {
			free(img_data);
			img_data = img_rgb;
		}

	} else if (!strcmp(img_fmt, IMG_FMT_MPI_JPEG)) {
		is_img_fmt_mpi_img = 1;
		int *a = (int*)malloc(sizeof(int)*1);
		a[0] = atoi(img_file);
		img_data = (uint8_t*)a; // target_echn
		fsize = 4;
		shape[0] = shape[1] = shape[2] = 0;
	} else if (!strcmp(img_fmt, IMG_FMT_MPI_Y)){
		is_img_fmt_mpi_img = 1;
		int *a = (int*)malloc(sizeof(int)*1);
		a[0] = strtol(img_file,0,16);
		img_data = (uint8_t*)a; // target_video_window
		fsize = 4;
		shape[0] = snapshot_height;
		shape[1] = snapshot_width;
		shape[2] = 1;
	} else {
		cerr << "Unsupported img format\n";
		return -1;
	}

	TIC(start);

	multipart_request req;
#ifndef CLI_USE_UNIX
	http_client<boost_service> client;
#else
	http_client<unix_service> client;
#endif

	if (is_img_fmt_mpi_img && !strcmp(meta, "none"))
		cerr << "[EAIF CLIENT : WARNING] User should define meta for mpi jpeg for classification query!\n";

	if (!strcmp(meta, "none"))
		snprintf(meta, 1024, "{\"od\":[{\"obj\":{\"id\":0,\"rect\":[0,0,%d,%d],\"cat\":\"\",\"shaking\":0}}]}",
			shape[0], shape[1]);

	snprintf(time_spec, 32, "%lu", start.tv_sec * 100 + start.tv_nsec/10000000);

	const char* file_names[] = {
		"shape", "meta", "format", "data", "time"
	};

	req.host = host;
	req.port = port;
	req.query_str = query_api;
	req.fill_content(file_names[0], reinterpret_cast<const uint8_t*>(shape), sizeof(int)*3);
	req.fill_content(file_names[1], reinterpret_cast<const uint8_t*>(meta), strlen(meta));
	req.fill_content(file_names[2], reinterpret_cast<const uint8_t*>(img_fmt), strlen(img_fmt));
	req.fill_content(file_names[3], img_data, fsize);
	req.fill_content(file_names[4], reinterpret_cast<const uint8_t*>(time_spec), strlen(time_spec));

	string shost(host);
	ret = client.connect(shost, port);
	assert(ret == 0);
	int size;
	sendmsg = req.to_str(size);
	TIC(start);
	ret = client.post(sendmsg, size);
	TOC("Response: ", start);

	delete[] sendmsg;

	free(img_data);
	sendmsg = nullptr;
	img_data = nullptr;
	assert(ret);
	return 0;
}
