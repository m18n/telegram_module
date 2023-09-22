#pragma once
#include "curl_wrapper.h"
class connector_manager {
  private:
  int my_id=-1;
public:
  connector_manager() {}
  std::string get_auth_code() {
    std::string code = "0";
    while(code=="0"){
        try {
            
            json jcode = cw.get_page_json("/api/telegram/command/"+std::to_string(my_id)+"/getauthcode");
            code = jcode["code"];
        } catch(const json::exception &e){
        }
    }
    if(code=="-1"){
      this->~connector_manager();
      exit(1);
    }
    return code;
  }
  
  int get_my_id(){
    if(my_id==-1){
      int code =-1;
      while(code==-1){
          try {
              
              json jcode = cw.get_page_json("/api/telegram/getid");
              code = jcode["id"];
          } catch(const json::exception &e){
          }
      }
      this->my_id=code;
    }
    return this->my_id;
  }
  ~connector_manager(){
    std::string code ="";
    while(code==""){
        try {
            code = cw.get_page("/api/telegram/command/"+std::to_string(my_id)+"/exit");
        } catch(const json::exception &e){
        }
    }
    std::cout<<"CODE: "<<code<<"\n";
  }
private:
  curl_wrapper cw;
};