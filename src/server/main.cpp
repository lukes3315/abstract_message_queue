// A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cereal/archives/binary.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <common/IpcInterface.hpp>

using namespace boost::interprocess;

int main (int argc, char *argv[])
{
  boost::interprocess::message_queue::remove("SERVER_MQ");
  IpcInterface<true, EXCHANGEABLE_TYPES> server_ipc("SERVER_MQ", 1000);

  server_ipc.registerCallback(
        [](common_types::DataType1 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << " " << data.string_ << std::endl;
        },[](common_types::DataType2 & data){
        std::cout << "RECEIVING: " << typeid(data).name()  << " " << data.data << std::endl;
        },[](common_types::DataType3 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](common_types::DataType4 & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](int & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](size_t & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](double & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](float & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
        },[](std::string & data){
        std::cout << "RECEIVING: " << typeid(data).name() << std::endl;
       }
  );


  return 0;
}
