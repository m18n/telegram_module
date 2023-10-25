#pragma once
#include"curl/curl.h"
#include<iostream>
#include <nlohmann/json.hpp>
#define IP_MANAGER "127.0.0.1:3000"
using t_json = nlohmann::json;
struct memory {
  char *response;
  size_t size;
};
 
class curl_wrapper{
private:
CURL* get_curl(){
    __thread static CURL* curl=nullptr;
        if(curl==nullptr){
            curl=curl_easy_init();
            curl_handels.push_back(curl);
        }
    return curl;
}
public:
    curl_wrapper() {
       
    }
   
    ~curl_wrapper() {
        
        for(int i=0;i<curl_handels.size();i++){
            if(curl_handels[i]){
                curl_easy_cleanup(curl_handels[i]);
            }
        }
       
    }
    
    std::string get_page(const std::string& url) {
        CURL* curl_handle=get_curl();
        std::string path=IP_MANAGER+url;
        std::string downloadedData;
       

        if (curl_handle) {
            curl_easy_setopt(curl_handle, CURLOPT_URL, path.c_str());
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl_handle, CURLOPT_POST, 0);
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, NULL); 
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, NULL);
            curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, NULL); 
            curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &downloadedData);
            CURLcode res = curl_easy_perform(curl_handle);
            
        }
        
        return downloadedData;
    }
    
    std::string get_page(const std::string& url,std::string json) {
        CURL* curl_handle=get_curl();
        std::string path=IP_MANAGER+url;
        std::string downloadedData;
        
        if (curl_handle) {

            curl_easy_setopt(curl_handle, CURLOPT_URL, path.c_str());
            curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json.c_str());
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, -1L);
            curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 0);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &downloadedData);

            CURLcode res = curl_easy_perform(curl_handle);
            
            // if (res != CURLE_OK) {
            //     std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
            // }
            curl_slist_free_all(headers);
        }
       
        return downloadedData;
    }
    t_json get_page_json(const std::string& url) {
        std::string downloadedData=get_page(url);
        t_json jsondata;
        try
        {
            jsondata = t_json::parse(downloadedData);
        }
        catch(const t_json::exception &e)
        {
            
        }
        return jsondata;
    }
    t_json get_page_json(const std::string& url,std::string json) {
        std::string downloadedData=get_page(url,json);
        t_json jsondata;
        try
        {
            jsondata = t_json::parse(downloadedData);
        }
        catch(const std::exception &e)
        {
               
        }
        return jsondata;
    }
private:
    
    std::vector<CURL*> curl_handels;


static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string *str = static_cast<std::string*>(userdata);
    size_t total_size = size * nmemb;
    str->append(ptr, total_size);
    return total_size; 
}
};