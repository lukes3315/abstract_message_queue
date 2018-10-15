#include <iostream>
#include <common/IpcInterface.hpp>
#include <thread>

int main(void){

    // Client instantiation.
    // Meta-false is for telling object remove MQ's on destructor.
    IpcInterface<false, EXCHANGEABLE_TYPES> client_mq;

    // Async testing
    std::thread t([&client_mq]{
        client_mq.passToMessageQueue<std::string>("Test / 1");
        client_mq.passToMessageQueue<std::string>("Test / 2");
        client_mq.passToMessageQueue<std::string>("Test / 3");
        client_mq.passToMessageQueue<std::string>("Test / 4"); 
    });

    // Async testing
    std::thread t2([&client_mq]{
        client_mq.passToMessageQueue<std::string>("Test / 5");
        client_mq.passToMessageQueue<std::string>("Test / 6");
        client_mq.passToMessageQueue<std::string>("Test / 7");
        client_mq.passToMessageQueue<std::string>("Test / 8");    
    });


    client_mq.passToMessageQueue<std::string>("AWESOME?./");
    client_mq.passToMessageQueue<std::string>("YES!");

    common_types::DataType1 datatype;
    datatype.string_="Testing ";

    client_mq.passToMessageQueue(datatype);

    // Testing rvalue const int & data passing
    client_mq.passToMessageQueue(4);
    client_mq.passToMessageQueue(10);
    // Testing size_t registered type.
    size_t data=100;
    client_mq.passToMessageQueue(data);
    // Testing int resgistered type.
    int d = 100;
    client_mq.passToMessageQueue(d);


    // Custom object registration.
    struct data_type_test{
        int i;
        void print(){
            std::cout << "THIS IS ME: " << i << std::endl;
        }
    };

    // Testing registering custom objects with custom values on server.
    data_type_test d_i;
    // Assigning i to 50.
    d_i.i=50;
    client_mq.registerObject(d_i);

    data_type_test d_i2;
    // Assigning i to 30.
    d_i2.i=30;
    client_mq.registerObject(d_i2);

    data_type_test d_i3;
    // Assigning i to 22220.
    d_i3.i=22220;
    client_mq.registerObject(d_i3);

    data_type_test d_i4;
    // Assigning i to 40.
    d_i4.i=40;
    client_mq.registerObject(d_i4);

    data_type_test d_i5;
    // Assigning i to 50.
    d_i5.i=50;
    client_mq.registerObject(d_i5);

    // Testing retrieving custom data from backend and calling method on registered object.
    // This will print out all the different values assigned to i.
    while (client_mq.retrieveObject(d_i5)){
        d_i5.print();
    }

    // Joining threads, prevent crash when ipcinterface goes out of scope.
    t.join();
    t2.join();
    return 0;
}