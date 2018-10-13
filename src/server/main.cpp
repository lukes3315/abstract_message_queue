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
        [](const common_types::DataType1 & data){
          std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;
        }
        , [](const common_types::DataType2 & data){
          std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;
       }, [](const common_types::DataType3 & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;
       }, [](const common_types::DataType4 & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;

       }, [](const int & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;

       }, [](const size_t & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;

       }, [](const double & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;

       }, [](const float & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;

       }, [](const std::string & data){
        std::cout << "RECEIVING: " << __PRETTY_FUNCTION__ << std::endl;
       }
  );


  return 0;
}
