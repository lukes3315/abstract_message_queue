## General explanation

Note: If you run in any issues, please just file an issue on github.

The purpose of the project is to leverage C++17's to guarantee type abstraction for server-client communication<br/>
using message_queues (in this case boost's message queues (since they work on linux and windows)).<br/>
The server runs in one termninal, the client in another. The server waits for incoming client connections.<br/>

The server can:
```
- receive any type of data (view code to see implementation) and process it in it's own memory space.
- store any type of data.
- retrieve stored data.
```
The client can:
```
- send data to server for processing.
- send data to server for storage.
- query data to be retrieved from server.
```
## Setup

First you are required to [install cmake](https://cmake.org/install/).

For Windows:<br/>
First, you'll need to [install Visual Studio](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=15#).
Then [download boost](https://www.boost.org/users/download/)<br/>and put it in the cloned directory.<br/>
Go in the boost directory and run:
```
bootstrap.bat
```
When this process completes, run:
```
.\b2
```

For Mac:<br/>
Download boost:<br/>
https://www.boost.org/users/download/<br/>
Decompress boost.
```
tar -xzf boost_1_68_0.tar.gz # Decompressed boost.
```
Change directory to boost's.
```
cd boost_1_68_0
```
Run boostrapping.
```
./bootstrap.sh
```
Compile boost.
```
./b2 toolset=clang threading=multi runtime-link=static  link=static cxxflags="-stdlib=libc++ -DBOOST_DISABLE_ASSERTS" linkflags="-stdlib=libc++" address-model=64
```
This will generate the appropriate dependencies for boost.

You're all set!

## Running it

First off, run `cmake`:
```
cmake .
```

If you are on Windows just open the .vxproj file with Visual Studio and run the program.

Starting here it's for Mac only, Windows users can directly jump to the output at the end.
Then, on Mac run `make`:
```
make
```

From there, two executables: `server` and `client` were generated.

You'll need to first run the server in one terminal:
```
./server
```

Always run the `server` first as he is the one creating the message queues.

Then to test out the `client`:
```
./client
```

To ensure it worked, this should be the output:
```
THIS IS ME: 50
THIS IS ME: 30
THIS IS ME: 22220
THIS IS ME: 40
THIS IS ME: 50
```

Which is essentially the client registering data to the server, retrieving it and calling the associated object.

If you run in any issues, please just file an issue on github.

## Design & Implementation

The initial idea was to have a system that allowed IPC that was as flexible as possible.<br/>
My approach was to have as much type abstraction as I could in a generic interface (e.g. `IpcInterface.hpp`).

The main problem with message queues is the following:
```
Client Data -> unmarshalling -> Pipe -> Server -> marshalling
```

Which means the Server needs to know how to marshall incoming data and call the respective<br/>
callback or execute the proper method associated to the marshalled data type that was retrieved<br/>
from the `pipe`.<br/>

First off, the server and the client NEED to know what will be the data types exchanged to have the<br/>
capacity to respond adequately to and from. From this I decided to start-off with a `Macro` to pass<br/>
in to the `IpcInterface`, as follows:<br/>
Server main.cpp:
```
  IpcInterface<true, EXCHANGEABLE_TYPES> server_ipc;
```
Client main.cpp:
```
    IpcInterface<false, EXCHANGEABLE_TYPES> client_mq;
```
The `Macro` definition:
```

// Custom test data types.
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
```
This `Macro` expands at compile time in the `IpcInterface` constructor:

```
    IpcInterface(const size_t & mq_size = 1000):receiver_message_queue_(boost::interprocess::open_or_create, receiver_mq_name_, mq_size, MQ_MSG_SIZE),
                                                output_message_queue_(boost::interprocess::open_or_create, output_mq_name_, mq_size, MQ_MSG_SIZE){
        // Based on the meta index of the EXCHANGEABLE_TYPES, run variadic expansion on every type and store it in a boost variant (to handle all possible function return types), and store the variant
        // in the std::vector of variants. Every type now has a decoder indexed at the same meta index it's located in.
        (variadic_decoder_.push_back(boost::variant<std::function<data_types(char*)> ... >( std::function<data_types(char*)>([=](char* data) -> data_types {return decodeData<data_types>(data);}))), ...);
    }
```

Before explaining the above line: when the client writes data to the `pipe` I add the meta-index as a 1 - byte meta-data to<br/> the pipped data. So that when the unmarshalled data arrives on the server side, the server knows exactly what <br/>
marshalling method to call and return the adequate type to finally pass that data to the proper callback.<br/>
This is what is happening:

```
Data In -> unmarshalling process -> 1 byte meta index header added -> Pipe -> server -> meta index parsing -> marshalling -> callback
```


Now, what is happening is that all the possible data decoding functions are getting added to a `std::vector`<br/>
in the order of the compile time expansion, which means the previously stored meta index as part of the meta data <br/>
can be used to retrieve the decoding function that was stored during object construction.<br/>

So now when we want to send data from the client to server:<br/>
Server code:
```
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
```
This code allows you to register `lambdas` or `std::functions` to receive all the marshalled data.<br/>

On the client:
```
        client_mq.passToMessageQueue<std::string>("Test / 1");
        size_t data=100;
        client_mq.passToMessageQueue(data);
        client_mq.passToMessageQueue(4);
        client_mq.passToMessageQueue<std::string>("Test / 4"); 
```

This is sending a string, unmarshalling it, passing to the pipe, marshalling and executing the proper callback.<br/>

So now the entire decoding system is actually a very short function:<br/>
```
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
```

This can handle as many data types as we want to be passed on generically only using a `Macro` (the limitation is the tuple<br/> which can only handle 10 meta-types). I think the `Macro` could be done in another way as well.<br/>

Now we have a generic pipeline for data communcation setup, we are going to move on the next steps, registering and <br/>
retrieving custom data types.

Here, I had to add two communication through a second `message_queue`. This `message_queue` enables the server to reply<br/>
back to client when a retrieval request is made.<br/>

For off, we need to register data types, what I chose is to leverage the basic reflection system that C++ offers, <br/>
essentially `<type_traits>`. This allows to extract the exact data type name during compilation. The way I chose to implement<br/> the registration was to use the reflected data type as a key with its respective unmarshalled data.<br/>
To handle multiple data types, I store a `std::queue` of unmarshalled data.<br/>

What happens here:
```
Client Data -> command + typeid(data).name + unmarshalled data -> Pipe -> Server -> storage in std::unordered_map<string, std::queue<std::string> > 
```

Then when I want to retrieve the data type, the same happens, except that the client is expecting a response:
```
Client Data -> command + typeid(data).name -> Pipe -> Server -> search in std::unordered_map<string, std::queue<std::string> > -> Pipe -> Client
```

You can see here the need for the secondary `message_queue` to handle server responses on the client side.

If you have any questions on the implementation please let me know in issues!

Thanks!
Cheers.
