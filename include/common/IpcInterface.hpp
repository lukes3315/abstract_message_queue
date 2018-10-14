#ifndef IPC_INTERFACE
#define IPC_INTERFACE

#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <common/Helper.hpp>
#include <string.h>
#include <type_traits>
#include <boost/variant.hpp>
#include <queue>

#define MQ_MSG_SIZE 1000

// Common types, server needs to know certain data types to be able to decode them on the backend.
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

// Supported data types for client/server runtime. (Not storage!)
#define EXCHANGEABLE_TYPES common_types::DataType1, common_types::DataType2, common_types::DataType3, common_types::DataType4, int, size_t, double, float, std::string

template<bool dtor_destroy_mq, typename ... data_types>
class IpcInterface{
    private:
    // Message Queue to receive data from client to server.
    boost::interprocess::message_queue receiver_message_queue_;
    // 2 way communication, from server back to client.
    boost::interprocess::message_queue output_message_queue_;

    // Named message queues.
    static constexpr char receiver_mq_name_[] = "MQ_SERVER";
    static constexpr char output_mq_name_[] = "MQ_SERVER_OUT";
    const size_t mq_size_;
    bool running_{true};
    // Storing type-based decoders in vector to know which type to decode.
    std::vector<boost::variant<std::function<data_types(char*)>...> > variadic_decoder_;
    // Storing callbacks from server in tuple.
    std::tuple<std::function<void(data_types & msg_data)>...> callbacks_;
    // Storing data types in queues, so can store multiple instance of same object and just pop the queue as you retrieve them.
    std::unordered_map<std::string, std::queue<std::string> > registered_data_;

    public:
    
    // Removes server receiving queue.
    static void removeReceiverQueue(){
        boost::interprocess::message_queue::remove(receiver_mq_name_);
    }
    // Removes client communication queue.
    static void removeOutputQueue(){
        boost::interprocess::message_queue::remove(output_mq_name_);
    }

    // Data decoder method store in vector.
    template<typename data_type>
    data_type decodeData(char* data){
        data_type d;
        strncpy((char*)&d, (char*)data, sizeof(data_type));
        return d;
    }

    // Overload for data decoding.
    template<typename data_type>
    void decodeData(char* data, data_type & type){
        strncpy((char*)&type, (char*)data, sizeof(data_type));
    }

    
    IpcInterface(const size_t & mq_size = 1000):receiver_message_queue_(boost::interprocess::open_or_create, receiver_mq_name_, mq_size, MQ_MSG_SIZE),
                                                output_message_queue_(boost::interprocess::open_or_create, output_mq_name_, mq_size, MQ_MSG_SIZE),
                                                mq_size_(mq_size){
        // Based on the meta index of the EXCHANGEABLE_TYPES, run variadic expansion on every type and store it in a boost variant (to handle all possible function return types), and store the variant
        // in the std::vector of variants. Every type now has a decoder indexed at the same meta index it's located in.
        (variadic_decoder_.push_back(boost::variant<std::function<data_types(char*)> ... >( std::function<data_types(char*)>([=](char* data) -> data_types {return decodeData<data_types>(data);}))), ...);
    }

    // Register lambda (or not) callbacks to main function for server.
    void registerCallback(const std::function<void(data_types & msg_data)> &... registered_callback){
        // Data size received from the queue.
        std::size_t received_size{0};
        // Message priority passed in.
        unsigned int msg_priority;
        // Incoming data size set by MQ_MSG_SIZE during pre-processing phase.
        char incoming_data[MQ_MSG_SIZE];
        // Create tuple to store callbacks and pass them to required visitor object.
        callbacks_=std::make_tuple(registered_callback...);
        while (running_){
            // Set all data to zero in incoming data to guarantee data cleanness.
            memset((void*)&incoming_data[0], 0, MQ_MSG_SIZE);
            // Blocking message queue receiver.
            receiver_message_queue_.receive((void*)&incoming_data[0], MQ_MSG_SIZE, received_size, msg_priority);
            const std::string data(&incoming_data[0], received_size);            
            if (!dataProcessing(data)){
                char idx=data[0];
                Helper::visitor_pattern<data_types...> visitor((char*)&data.c_str()[1], callbacks_);
                boost::apply_visitor(visitor, variadic_decoder_[idx]);
            }
        }
    }

    template<typename data_type, int msg_priority = 1>
    void passToMessageQueue(const data_type & msg_data){
        char buf[sizeof(data_type)+1];
        memset(&buf[0], 0, sizeof(data_type));
        buf[0]=(char)Helper::get_index<data_type, EXCHANGEABLE_TYPES>();
        strncpy(&buf[1], (char*)&msg_data, sizeof(data_type));
        receiver_message_queue_.send((char*)&buf[0], sizeof(data_type)+1, msg_priority);
    }

    // template<typename data_type, int msg_priority = 1>
    template<typename data_type, int msg_priority = 1>
    void registerObject(const data_type & data)
    {
        std::string demarshalled((char*)&data, sizeof(data_type));
        demarshalled="_register_:"+std::string(typeid(data_type).name())+":"+demarshalled;
        receiver_message_queue_.send(demarshalled.c_str(), demarshalled.size(), msg_priority);
    }

    template<typename data_type, unsigned int msg_priority=1>
    bool retrieveObject(data_type & data){
        std::string msg{"_retrieve_:"+std::string(typeid(data_type).name())};
        receiver_message_queue_.send(msg.c_str(), msg.size(), msg_priority);
        char incoming_data[MQ_MSG_SIZE];
        std::size_t received_size{0};
        unsigned int msg_priority_out;
        memset((void*)&incoming_data[0], 0, MQ_MSG_SIZE);
        output_message_queue_.receive((void*)&incoming_data[0], MQ_MSG_SIZE, received_size, msg_priority_out);
        std::string str_data(&incoming_data[0], received_size);
        if (str_data.find("EMPTY") == std::string::npos){
            decodeData((char*)str_data.c_str(), data);
            return true;
        }
        return false;
    }

    bool dataProcessing(const std::string & data){
        if (data.find("_register_")!=std::string::npos){
            auto idx=data.find(":");
            if (idx != std::string::npos){
                    auto current_data = data.substr(idx+1);
                    auto idx_end = current_data.find(":");
                    const auto & key = current_data.substr(0, idx_end);
                    auto it = registered_data_.find(key);
                    if (it == registered_data_.end()){
                        std::queue<std::string> queued_data;
                        queued_data.push(current_data.substr(idx_end+1, current_data.size()));
                        registered_data_.insert(std::pair<std::string, std::queue<std::string> >(key, queued_data));
                    }
                    else{
                        it->second.push(current_data.substr(idx_end+1, current_data.size()));
                    }
                }
            return true;
        }
        else if (data.find("_retrieve_") != std::string::npos){
            auto idx=data.find(":");
            if (idx != std::string::npos){
                auto it = registered_data_.find(data.substr(idx+1, data.size()));
                if (it != registered_data_.end() && !it->second.empty()){
                    auto data = it->second.front();
                    it->second.pop();
                    output_message_queue_.send((char*)data.c_str(), data.size(), 1);
                }
                else{
                    output_message_queue_.send("EMPTY", 5, 1);
                }
            }
            return true;
        }
        return false;
    }


    ~IpcInterface(){
        if (dtor_destroy_mq){
            removeReceiverQueue();
            removeOutputQueue();
        }
    }
};

#endif