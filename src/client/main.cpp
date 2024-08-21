#include <iostream>

#include "chatclient.hpp"

int main(int argc, char *argv[])
{
    if(argc!=3)
    {
        fprintf(stdout,"Usage:%s ip port\n",argv[0]);
        exit(-1);
    }

    ChatClient client(argv[1],atoi(argv[2]));
    client.start();

    return 0;
}