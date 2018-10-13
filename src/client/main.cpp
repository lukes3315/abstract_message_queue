#include <iostream>
#include <cereal/archives/binary.hpp>
#include <common/IpcInterface.hpp>

int main(void){
    IpcInterface<false, EXCHANGEABLE_TYPES> client_mq("SERVER_MQ", 1000);

    client_mq.passToMessageQueue<1, std::string>("AWESOME./");
    client_mq.passToMessageQueue<1, std::string>("WTF IS GOING ON");

    client_mq.passToMessageQueue<1, std::string>("HELLO");

    client_mq.passToMessageQueue(4);
    client_mq.passToMessageQueue(10);
    size_t data=100;
    client_mq.passToMessageQueue(data);
    int d= 100;
    client_mq.passToMessageQueue(d);
    // struct test_{
    //     int a=0;
    //     int b=0;
    //     double sdasd=32;
    // };
    // test_ t;
    // client_mq.passToMessageQueue(t);


    return 0;
}