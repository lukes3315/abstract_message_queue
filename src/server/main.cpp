// A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <common/IpcInterface.hpp>

using namespace boost::interprocess;

int main (int argc, char *argv[])
{
  // Removing previous queues to ensure to old is in it.
  IpcInterface<true, EXCHANGEABLE_TYPES>::removeReceiverQueue();
  IpcInterface<true, EXCHANGEABLE_TYPES>::removeOutputQueue();
  // Instantiate server.
  // Meta-true is for telling object remove MQ's on destructor.
  IpcInterface<true, EXCHANGEABLE_TYPES> server_ipc;

  // Register server callbacks to handle all possible data type in server-client communications.
  // Blocking call.
  server_ipc.registerCallback(
        [](common_types::DataType1 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " " << data.string_ << std::endl;
        },[](common_types::DataType2 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " " << data.data << std::endl;
        },[](common_types::DataType3 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](common_types::DataType4 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](int & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " "  << data << std::endl;
        },[](size_t & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " "  << data << std::endl;
        },[](double & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " "  << data << std::endl;
        },[](float & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " "  << data << std::endl;
        },[](std::string & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " "  << data << std::endl;
       }
  );
  return 0;
}
