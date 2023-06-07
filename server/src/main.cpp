#include "websockets/websocketServer.hpp"

void exec_server();

int main(int argc, char** args)
{
    exec_server();
    return 0;
}

void exec_server()
{
    int port = 8080;

    try {
    broadcast_server server_instance;

    // Start a thread to run the processing loop
    thread t(bind(&broadcast_server::process_messages,&server_instance));

    // Run the asio loop with the main thread
    printf("Running on port %d\n",port);
    server_instance.run(port);

    t.join();

    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}
