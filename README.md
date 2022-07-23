## Simple cURL request wrapper C++
This project is a very simple 1 header solution to make easy http requests via cURL.

### Features
- Easy to use
- Wrapping of cURL functions
- Usage of maps 
- Cookie wrapping
- Usage of modern C++ features

### Usage
Requires c++17 standard and above
An example can be seen in the [example.cpp](example.cpp) file.
```cpp
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
```

the types `headers_t` and `cookies_t` are simple maps that can be modified and used as you wish.
the api will process all the data and construct a cURL request.

The object can be reused for multiple requests and the headers and cookies will be kept in memory.

### Linking
The linking of curl is up to the user, if the curl is not directly linked on the system and there is a manual installed version of curl, define the path to it via `#define MANUAL_CURL_PATH "path/to/curl"` the rest of the linking (such as libraries like `libcurl`) are left to the user.
