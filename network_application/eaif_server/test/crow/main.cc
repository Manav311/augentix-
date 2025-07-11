#include <stdlib.h>

#include <iostream>

#include "crow.h"


class CrowApp
{
    public:
        crow::SimpleApp app;
        void regis_app() {
            CROW_ROUTE(app, "/update")
                .methods("GET"_method)
                ([](const crow::request& req){
                 std::cout << "[CROW TEST] hi\n";
                 return crow::response{"{\"success\":1}\n"};
                 });
            return;

        }
};



int test(char *addr, int port)
{
    CrowApp *serv = new CrowApp();
    serv->regis_app();
    serv->app.bindaddr(addr).port(port).concurrency(1).run();
    return 0;
    
}

int main(int argc, char **argv)
{
    test(argv[1], atoi(argv[2]));
    return 0;
}
