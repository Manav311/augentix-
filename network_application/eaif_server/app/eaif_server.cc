#include <cassert>
#include <getopt.h>

//#include "eaif_engine.h"
#include "eaif_service.h"
#include "eaif_trc.h"


using namespace eaif;
using namespace std;

void print_ops(void)
{
    printf("\t ===== help ====\n"
           "\t --host <host ip>\n"
           "\t --port <port>\n"
           "\t --c <config file>\n");
}

int main(int argc, char **argv)
{


    static struct option long_options[] =
    {
        // inference type
        {"host", required_argument, 0,'h'},
        {"port",  required_argument, 0, 'p'},
        {"config",  required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    string host("0.0.0.0");
    string config("config.ini");
    int port = 40080;

    while (1)
    {
          /* getopt_long stores the option index here. */
          int option_index = 0;
          //char *tok;
          //int i;

          int c = getopt_long (argc, argv, "h:p:c:",
                           long_options, &option_index);

          /* Detect the end of the options. */
          if (c == -1)
            break;

          switch (c)
          {
              case 'h':
                host = string(optarg);
                break;
              case 'p':
                port = atoi(optarg);
                break;
              case 'c':
                config = string(optarg);
                break;
              default:
                cout << "unknown parameters!\n"; 
                print_ops();
                return -1;
          }
    }

    HttpServiceFactory a;
    shared_ptr<HttpService> app = a.GetServiceInstance();
    assert(app);

    int ret = app->RegisterApp(config);
    if (ret != EAIF_SUCCESS)
      return -1;
    app->Run(host, port);
    return 0;
}

	
