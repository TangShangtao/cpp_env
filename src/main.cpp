#include "dbapi.h"

int main()
{
    std::string host_ip = "10.243.247.0";
    std::string user_name = "tangshangtao";
    std::string password = "Tt1234567890";
    std::string db_name = "research";

    DBApi dbapi(host_ip, user_name, password, db_name);
}