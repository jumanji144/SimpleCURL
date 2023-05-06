#pragma once
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <fstream>
#ifdef MANUAL_CURL_PATH // manually linking
#include MANUAL_CURL_PATH
#else
#include <curl/curl.h>
#endif

typedef std::map<std::string, std::string> headers_t;
typedef headers_t cookies_t;

static size_t _m_writeFunction(void* ptr, size_t size, size_t nmemb, std::vector<uint8_t>* userdata)
{
    uint8_t* data = (uint8_t*)ptr;
    for (size_t i = 0; i < nmemb; i++)
    {
        userdata->push_back(data[i]);
    }

    return size * nmemb;
}

static size_t _m_fileWriteFunction(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    std::ofstream* ofs = static_cast<std::ofstream*>(userdata);
    size_t bytes_written = size * nmemb;
    ofs->write(static_cast<const char*>(ptr), bytes_written);
    return bytes_written;
}


struct Response
{
    CURLcode curlCode;
    int code;
    std::string message;
    std::string data;
    std::vector<uint8_t> rawData;
    std::vector<uint8_t> rawHeaders;
    headers_t headers;
    cookies_t cookies;
};

class Form {
    public:
        explicit Form(CURL* curl) {
            form = curl_mime_init(curl);
        };

        /**
         * @brief append a string to the form
         * @param name the name of the field
         * @param value the value of the field
        */
        void append_string(const std::string& name, const std::string& value) {
            auto part = append(name);
            curl_mime_data(part, value.c_str(), CURL_ZERO_TERMINATED);
        }

        /**
         * @brief append a file to the form
         * @param name the name of the field
         * @param path the path of the file
        */
        void append_file(const std::string& name, const std::string& path) {
            auto part = append(name);
            curl_mime_filedata(part, path.c_str());
        }

        /**
         * @brief append a file to the form
         * @param name the name of the field
         * @param path the path of the file
         * @param filename the filename of the file
        */
        void append_file(const std::string& name, const std::string& path, const std::string& filename) {
            auto part = append(name);
            curl_mime_filename(part, filename.c_str());
            curl_mime_filedata(part, path.c_str());
        }

        /**
         * @brief append a part to the form
         * @param name the name of the field
         * @return the part
         * @note you can use this to set custom field data
        */
        curl_mimepart* append(const std::string& name) {
            auto part = curl_mime_addpart(form);
            curl_mime_name(part, name.c_str());
            return part;
        }

        curl_mime* get() {
            return form;
        }
    
    protected:
        friend class Request;
        curl_mime* form = nullptr;
};

class Request
{

public:
    Request(std::string url) : url(url), data(""), headers() {}

    Request(std::string url, std::string data) : url(url), data(data), headers() {}

    Request(std::string url, std::string data, headers_t headers) : url(url), data(data), headers(headers) {}

    ~Request()
    {
        if (curl_headers)
            curl_slist_free_all(curl_headers);
        if (curl)
            curl_easy_cleanup(curl);
    }

    /**
     * @brief initalize the curl backend, must be called before any other method
     * @return true when success
     */
    int initalize()
    {
        CURL* localCurl = curl_easy_init();
        this->curl = localCurl;
        return this->curl != nullptr;
    }
    /**
     * @brief execute the request with the method POST
     * @return the response of the request
     */
    Response post()
    {
        this->prepare();
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        return this->execute();
    }
    /**
     * @brief execute the request with the method GET
     * @return the response of the request
     */
    Response get()
    {
        this->prepare();
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        return this->execute();
    }
    /**
     * @brief execute the request with the method defined (must be a valid HTTP 1.1 method)
     * @param method the method to use
     * @return the response of the request
     */
    Response request(std::string method)
    {
        this->prepare();
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        return this->execute();
    }
    /**
    * @brief Download a file from the specified URL using a GET request and save it to the given file path
    * @param file_path The path where the downloaded file should be saved
    * @return The result of the cURL request
    */
    int download_file(const std::string& file_path)
    {
        std::ofstream ofs(file_path, std::ios::out | std::ios::binary);

        this->prepare();

        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _m_fileWriteFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        int result = curl_easy_perform(curl);

        ofs.close();

        return result;
    }
    /**
     * @brief set the url of the request
     * @param url the url of the request
     */
    void set_url(std::string url)
    {
        this->url = url;
    }
    /**
     * @brief set the data of the request
     * @param data the data of the request
     */
    void set_data(std::string data)
    {
        this->data = data;
    }
    /**
     * @brief set a header in the request
     * @param key the key of the header
     * @param value the value of the header
     */
    void set_header(std::string key, std::string value)
    {
        this->headers[key] = value;
    }
    /**
     * @brief set a cookie in the request
     * @param key the key of the cookie
     * @param value the value of the cookie
     */
    void set_cookie(std::string key, std::string value)
    {
        this->cookies[key] = value;
    }
    /**
     * @brief remove a header from the request
     * @param key the key of the header
     */
    void remove_header(std::string key)
    {
        this->headers.erase(key);
    }
    /**
     * @brief remove a cookie from the request
     * @param key the key of the cookie
     */
    void remove_cookie(std::string key)
    {
        this->cookies.erase(key);
    }
    /**
     * @brief return a modifiable map of the headers of the request
     * @return the map of the headers
     */
    const headers_t& get_headers() const
    {
        return headers;
    }
    /**
     * @brief return a modifiable map of the cookies of the request
     * @return the map of the cookies
     */
    const cookies_t& get_cookies() const
    {
        return cookies;
    }
    /**
     * @brief return the url of the request
     * @return the url
     */
    std::string get_url() const
    {
        return url;
    }
    /**
     * @brief return the data of the request
     * @return the data
     */
    std::string get_data() const
    {
        return data;
    }

private:
    std::string url;
    std::string data;
    headers_t headers{};
    cookies_t cookies{};
    curl_slist* curl_headers = nullptr;

    void prepare()
    {
#ifdef _VERBOSE
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // set headers
        prepareHeaders();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    void prepareHeaders()
    {
        // cleanup old headers in case this gets reused
        if (curl_headers)
            curl_slist_free_all(curl_headers);
        // redefine headers
        curl_slist* curl_headers = NULL;
        for (auto [key, value] : headers)
        {
            std::string header = key + ": " + value;
            curl_headers = curl_slist_append(curl_headers, header.c_str());
        }

        // prepare cookies
        std::string cookie_header = "Cookie: ";
        for (auto [key, value] : cookies)
        {
            cookie_header += key + "=" + value + "; ";
        }
        curl_headers = curl_slist_append(curl_headers, cookie_header.c_str());

        this->curl_headers = curl_headers; // save for later cleanup
        // set headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
    }
    Response execute()
    {
        Response response{};
        std::vector<uint8_t> responseData;
        std::vector<uint8_t> headerData;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _m_writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        CURLcode curlCode = curl_easy_perform(curl);

        // read into response data
        response.rawData = responseData;
        uint8_t* rawData = responseData.data();
        response.data = std::string(rawData, rawData + responseData.size());
        response.rawHeaders = headerData;
        uint8_t* rawHeaders = headerData.data();
        std::string headers = std::string(rawHeaders, rawHeaders + headerData.size());

        // get first line
        std::stringstream ss(headers);
        std::string line;
        std::getline(ss, line);

        int index = line.find(' ');
        std::string responsePart = line.substr(index + 1);

        index = responsePart.find(' ');
        std::string message = responsePart.substr(index + 1);
        std::string codeStr = responsePart.substr(0, index);

        int code = std::stoi(codeStr);

        response.code = code;
        response.message = message;

        // parse header string to vector of headers
        ss = std::stringstream(headers);
        while (std::getline(ss, line))
        {
            std::string key;
            std::string value;
            std::stringstream lineStream(line);
            std::getline(lineStream, key, ':');
            // consume 1 space
            std::getline(lineStream, value, ' ');
            // get actual value
            std::getline(lineStream, value, '\r');

            response.headers[key] = value;
        }

        if (response.headers.find("set-cookies") != response.headers.end()) {
            // parse response cookies
            std::stringstream ss(response.headers["set-cookies"]);
            std::string line;
            while (std::getline(ss, line))
            {
                std::string key;
                std::string value;
                std::stringstream lineStream(line);
                std::getline(lineStream, key, '=');
                // get actual value
                std::getline(lineStream, value, ';');

                response.cookies[key] = value;
            }
        }
        response.curlCode = curlCode;
        return response;
    }

    CURL* curl;
};
