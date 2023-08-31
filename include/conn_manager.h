#pragma once
#include "curl_wrapper.h"
class connector_manager {
public:
  connector_manager() {}
  std::string get_auth_code() {
    std::string code = "";
    while(code==""){
        try {
            
            json jcode = cw.get_page_json("/api/telegram/getauthcode");
            code = jcode["code"];
        } catch(const json::exception &e){
        }
    }
    if(code=="-1"){
      exit(1);
    }
    return code;
  }
  int get_my_id(){
    int code =-1;
    while(code==-1){
        try {
            
            json jcode = cw.get_page_json("/api/telegram/getid");
            code = jcode["id"];
        } catch(const json::exception &e){
        }
    }
    return code;
  }
private:
  curl_wrapper cw;
};