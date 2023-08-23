#pragma once
#include"curl/curl.h"
#include<iostream>
#include <nlohmann/json.hpp>
#define IP_MANAGER "127.0.0.1:3000"
using json = nlohmann::json;
class curl_wrapper{
public:
    curl_wrapper() {
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
    }

    ~curl_wrapper() {
        if (curl_handle) {
            curl_easy_cleanup(curl_handle);
        }
        curl_global_cleanup();
    }

    std::string get_page(const std::string& url) {
        std::string path=IP_MANAGER+url;
        std::string downloadedData;

        if (curl_handle) {
            curl_easy_setopt(curl_handle, CURLOPT_URL, path.c_str());
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &downloadedData);

            CURLcode res = curl_easy_perform(curl_handle);
            
            // if (res != CURLE_OK) {
            //     std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
            // }
        }

        return downloadedData;
    }
    json get_page_json(const std::string& url) {
        std::string downloadedData=get_page(url);
        json jsondata;
        try
        {
            jsondata = json::parse(downloadedData);
        }
        catch(const json::exception &e)
        {
            
        }
        return jsondata;
    }
private:
    CURL* curl_handle;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* user_data) {
        size_t real_size = size * nmemb;
        std::string* data = static_cast<std::string*>(user_data);
        data->append(static_cast<char*>(contents), real_size);
        return real_size;
    }
};