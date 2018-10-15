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

    // Internal running boolean, is only used in registerCallbacks.
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
                                                output_message_queue_(boost::interprocess::open_or_create, output_mq_name_, mq_size, MQ_MSG_SIZE){
        // Based on the meta index of the EXCHANGEABLE_TYPES, run variadic expansion on every type and store it in a boost variant (to handle all possible function return types), and store the variant
        // in the std::vector of variants. Every type now has a decoder indexed at the same meta index it's located in.
        (variadic_decoder_.push_back(boost::variant<std::function<data_types(char*)> ... >( std::function<data_types(char*)>([=](char* data) -> data_types {return decodeData<data_types>(data);}))), ...);
    }

    // Register lambda (or not) callbacks to main function for server.
    void registerCallback(const std::function<void(data_types & msg_data)> &... registered_callback){
        // Data size received from the queue.
        std::size_t received_size{0};
        // Message priority passed in.
        unsigned int msg_priority{0};
        // Incoming data size set by MQ_MSG_SIZE during pre-processing phase.
        char incoming_data[MQ_MSG_SIZE];
        // Create tuple to store callbacks and pass them to required visitor object.
        callbacks_=std::make_tuple(registered_callback...);
        while (running_){
            // Set all data to zero in incoming data to guarantee data cleanness.
            memset((void*)&incoming_data[0], 0, MQ_MSG_SIZE);
            // Blocking message queue receiver.
            receiver_message_queue_.receive((void*)&incoming_data[0], MQ_MSG_SIZE, received_size, msg_priority);
            // Input data to std::string to be able to run std::string algorithms on it.
            const std::string data(&incoming_data[0], received_size);            
            if (!dataProcessing(data)){
                // Meta index stored in de-marshalled data which allow us to know which decoder to run.
                char idx=data[0];
                // Find which variant of the decoder (which return type)
                Helper::visitor_pattern<data_types...> visitor((char*)&data.c_str()[1], callbacks_);
                // And call the registered_listener using the boost apply_visitor.
                boost::apply_visitor(visitor, variadic_decoder_[idx]);
            }
        }
    }

    // Send data from client to server. (Only EXCHANGEABLE_TYPES)
    template<typename data_type, int msg_priority = 1>
    void passToMessageQueue(const data_type & msg_data){
        char buf[sizeof(data_type)+1];
        memset(&buf[0], 0, sizeof(data_type));
        // Set meta-index from the i
        buf[0]=(char)Helper::get_index<data_type, EXCHANGEABLE_TYPES>();
        strncpy(&buf[1], (char*)&msg_data, sizeof(data_type));
        receiver_message_queue_.send((char*)&buf[0], sizeof(data_type)+1, msg_priority);
    }

    // Register custom C++ object from client to server.
    template<typename data_type, int msg_priority = 1>
    void registerObject(const data_type & data)
    {
        std::string demarshalled((char*)&data, sizeof(data_type));
        // Using basic C++ reflexion to generate key for unordered_map.
        demarshalled="_register_:"+std::string(typeid(data_type).name())+":"+demarshalled;
        // Send command to server with demarshalled data.
        receiver_message_queue_.send(demarshalled.c_str(), demarshalled.size(), msg_priority);
    }

    // Retrieve custom C++ object store on server and send it back to client.
    template<typename data_type, unsigned int msg_priority=1>
    bool retrieveObject(data_type & data){
        // Retrieval code.
        std::string msg{"_retrieve_:"+std::string(typeid(data_type).name())};
        // Send object type we want to retrieve from client to server.
        receiver_message_queue_.send(msg.c_str(), msg.size(), msg_priority);

        // Create response objects.
        char incoming_data[MQ_MSG_SIZE];
        std::size_t received_size{0};
        unsigned int msg_priority_out;
        memset((void*)&incoming_data[0], 0, MQ_MSG_SIZE);
        // Blocking response wait.
        output_message_queue_.receive((void*)&incoming_data[0], MQ_MSG_SIZE, received_size, msg_priority_out);
        // Receive response.
        std::string str_data(&incoming_data[0], received_size);
        // If response contains data will be something else than empty.
        if (str_data.find("EMPTY") == std::string::npos){
            // Decode data.
            decodeData((char*)str_data.c_str(), data);
            // The object was decoded and true returned to say the operation was successful.
            return true;
        }
        // No object of this type were stored.
        return false;
    }

    // Checking register and retrival commands of objects.
    bool dataProcessing(const std::string & data){
        // Register object into unordered_map.
        if (data.find("_register_")!=std::string::npos){
            // Parse object.
            auto idx=data.find(":");
            if (idx != std::string::npos){
                    // Extract key and data from string.
                    auto key_and_data = data.substr(idx+1);
                    // Extract idx_end, which is where the data segment starts.
                    auto idx_end = key_and_data.find(":");
                    // Extract key part.
                    const auto & key = key_and_data.substr(0, idx_end);
                    // Search for key in unordered_map.
                    auto it = registered_data_.find(key);
                    if (it == registered_data_.end()){
                        // If no key present, create key and queue data association.
                        std::queue<std::string> queued_data;
                        // Insert data segment in the queue.
                        queued_data.push(key_and_data.substr(idx_end+1, key_and_data.size()));
                        // Insert key and queue in unordered_map.
                        registered_data_.insert(std::pair<std::string, std::queue<std::string> >(key, queued_data));
                    }
                    else{
                        // Key exists, insert data segment in queue.
                        it->second.push(key_and_data.substr(idx_end+1, key_and_data.size()));
                    }
                }
            // Command found skip data decoding section.
            return true;
        }
        // Object retrieval.
        else if (data.find("_retrieve_") != std::string::npos){
            // Parse retrieve command.
            auto idx=data.find(":");
            if (idx != std::string::npos){
                // Find data associated with key in unordered_map.
                auto it = registered_data_.find(data.substr(idx+1, data.size()));
                if (it != registered_data_.end() && !it->second.empty()){
                    // Get object.
                    auto data = it->second.front();
                    // Pop queue.
                    it->second.pop();
                    // Send object back to blocked client.
                    output_message_queue_.send((char*)data.c_str(), data.size(), 1);
                }
                else{
                    // If no object is found, return EMPTY message to waiting client.
                    output_message_queue_.send("EMPTY", 5, 1);
                }
            }
            // Command found skip data decoding section.
            return true;
        }
        // If no commands have been found in the data, then continue to decode the default data type.
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