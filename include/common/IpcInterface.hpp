#ifndef IPC_INTERFACE
#define IPC_INTERFACE

#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <common/Helper.hpp>
#include <string.h>
#include <type_traits>
#include <variant>

#define MQ_MSG_SIZE 1000


namespace common_types{
    struct DataType1{
        std::string string_{"TEST_STRING"};
    };
    struct DataType2{
        int data{23};
    };
    struct DataType3{
        double data{2.3};
    };
    struct DataType4{
        size_t data{42};
    };
};

#define EXCHANGEABLE_TYPES common_types::DataType1, common_types::DataType2, common_types::DataType3, common_types::DataType4, int, size_t, double, float, std::string

template<bool dtor_destroy_mq, typename ... data_types>
class IpcInterface{
    private:
    boost::interprocess::message_queue internal_message_queue_;
    const std::string mq_name_;
    const size_t mq_size_;
    bool running_{true};
    std::vector<std::variant<std::function<data_types(char*)>...> > variadic_decoder_;
    
    public:
    
    template<typename data_type>
    data_type decodeData(char* data)
    {
        data_type d;
        strncpy((char*)&d, (char*)data, sizeof(data_type));
        return d;
    }
    
    IpcInterface(const std::string & mq_name, const size_t & mq_size):internal_message_queue_(boost::interprocess::open_or_create, mq_name.c_str(), mq_size, MQ_MSG_SIZE), mq_name_(mq_name), mq_size_(mq_size){\
        (variadic_decoder_.push_back(std::variant<std::function<data_types(char*)> ... >( std::function<data_types(char*)>([=](char* data) -> data_types {return decodeData<data_types>(data);}))), ...);
    }

    void registerCallback(const std::function<void(const data_types & msg_data)> &... registered_callback)
    {
        std::size_t received_size{0};
        unsigned int msg_priority;
        char incoming_data[MQ_MSG_SIZE];

        while (running_){
            memset((void*)&incoming_data[0], 0, MQ_MSG_SIZE);
            internal_message_queue_.receive((void*)&incoming_data[0], MQ_MSG_SIZE, received_size, msg_priority);
            char idx=incoming_data[0];
            const std::string data(&incoming_data[1], received_size-1);
            (register_callback(std::get<std::function<data_types(char*)> >(variadic_decoder_[idx])((char*)data.c_str())), ...);
        }
    }

    template<unsigned int msg_priority = 1>
    void passToMessageQueue(const std::string & msg_data){
	    internal_message_queue_.send(msg_data.c_str(), msg_data.size(), msg_priority);
    }

    template<unsigned int msg_priority = 1>
    void passToMessageQueue(std::string msg_data){
	    internal_message_queue_.send(msg_data.c_str(), msg_data.size(), msg_priority);
    }

    template<unsigned int msg_priority = 1, typename data_type>
    void passToMessageQueue(const data_type & msg_data){
        char buf[sizeof(data_type)+1];
        memset(&buf[0], 0, sizeof(data_type));
        buf[0]=(char)Helper::get_index<data_type, EXCHANGEABLE_TYPES>();
        strncpy(&buf[1], (char*)&msg_data, sizeof(data_type));
        internal_message_queue_.send((char*)&buf[0], sizeof(data_type)+1, msg_priority);
    }

    ~IpcInterface(){
        if (dtor_destroy_mq)
        {
            boost::interprocess::message_queue::remove(mq_name_.c_str());
        }
    }
};

#endif