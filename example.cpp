#include "curl_request.hpp"
#include <iostream>

int main() {
    // simple request to google.com
    Request request("https://www.google.com");
    request.initalize();
    request.set_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");
    request.set_header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    request.set_header("Accept-Language", "en-US,en;q=0.9");
    request.set_header("Accept-Encoding", "gzip, deflate, br");

    request.set_cookie("__cfduid", "d41d8cd98f00b204e9800998ecf8427e");
    request.set_cookie("_ga", "GA1.2.1809898981.1544009800");
    request.set_cookie("_gid", "GA1.2.1809898981.1544009800");
    request.set_cookie("_gat", "1");
    request.set_cookie("_hjid", "1544009800");
    request.set_cookie("_hjIncludedInSample", "1");
    Response response = request.get();

    // print data and headers
    std::cout << response.data << std::endl;
    for (auto[key, value] : response.headers) // print response headers
    {
        std::cout << key << ": " << value << std::endl;
    }
}

// websockets
#include <thread>

int websockets() {
    // create a request to the websocket server
    Request req("wss://ws.postman-echo.com/raw");

    WebSocket ws(req);

    // set an on message handler
    ws.setOnMessage([](auto ws, auto msg){
        std::cout << "Message: " << msg << std::endl;
    });

    ws.initialize();

    // connection thread
    std::thread t([&ws](){
        ws.connect();
    });

    while(ws.isConnected()) {
        // get input
        std::string input;
        std::getline(std::cin, input);

        // send input
        ws.send(input);
    }

    // wait for connection thread to finish
    t.join();
}