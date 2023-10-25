#pragma once
#include "curl_wrapper.h"
#include <unistd.h>
#include<iostream>
#include<thread>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

#define NAME_SERVER "telegram"
std::string GetLocalIP() ;
struct return_data {
  t_json json_send;
  
  std::string server_hash;
  int respon_id = -1;
  void (*callback)(t_json jsonsend,t_json json_answer)=NULL;
};
void init_return_data(return_data* data);
class connector_manager;
struct handler{
  std::string nameobj="";
  void (*callback)(connector_manager* conn,t_json json_req)=NULL;
};
struct event{
  void(*handling)(t_json json)=NULL;
  t_json json;
};
class connector_manager {
private:
  std::string hash_id = "";
  std::string server_hash;
  time_t start_time;
  std::string local_ip;
private:
  void add_returns(return_data d) {
    for (int i = 0; i < returns.size(); i++) {
      if (returns[i].respon_id == -1) {
        returns[i] = d;
        return;
      }
    }
    returns.push_back(d);
  }
  void delete_return(return_data d) {
    for (int i = 0; i < returns.size(); i++) {
      if (returns[i].respon_id == d.respon_id) {
        returns[i].respon_id = -1;
        return;
      }
    }
  }
  int search_returns(int respon_id,std::string server_hash){
    for(int i=0;i<returns.size();i++){
      if(returns[i].respon_id==respon_id&&returns[i].server_hash==server_hash){
        return i;
      }
    }
    return -1;
  }
  void get_my_id() {
  
      std::string hash_id ="";
      std::string server_hash;
      
      t_json json;
      std::string name_server=NAME_SERVER;
      json["$time"]=std::to_string(start_time);
      json["$ip"]=local_ip;
      while (hash_id == "") {
        try {
          t_json jcode = cw.get_page_json("/api/client/"+name_server+"/getid",json.dump());
          hash_id = jcode["$hash_id"];
          server_hash=jcode["$server_hash"];
          if(hash_id==""){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
          }
        } catch (const t_json::exception &e) {
         //std::cout<<"ERROR: "<< e.what()<<"\n";
        }
      }
     
      this-> hash_id= hash_id;
      this->server_hash=server_hash;
      std::cout<<"SERVER ID: "<<server_hash<<" ID: "<<hash_id<<"\n";
    
  }

public:
  connector_manager() {
    
    start_time=time(nullptr); 
    local_ip=GetLocalIP();
    returns.resize(25);
    get_my_id();
    start_loop();
  }
  // std::string get_auth_code() {
  //   std::string code = "0";
  //   while(code=="0"){
  //       try {

  //           json jcode =
  //           cw.get_page_json("/api/telegram/command/"+std::to_string(my_id)+"/getauthcode");
  //           code = jcode["code"];
  //       } catch(const json::exception &e){
  //       }
  //   }
  //   if(code=="-1"){
  //     this->~connector_manager();
  //     exit(1);
  //   }
  //   return code;
  // }

  void send(t_json json, void (*callback)(t_json jsonsend,t_json jsonanswer)) {
    
    int id = -1;
    std::string server_id;
    std::string ns=NAME_SERVER;
    
    while (id == -1) {
      try {
        t_json jsonres = cw.get_page_json(
            "/api/send/"+server_hash+"/"+ns+"/command/" + hash_id+"/event", json.dump());
        //std::cout<<"RES: "<<jsonres.dump()<<"\n";
        if(jsonres.contains("$error")){
          get_my_id();
        }else{
          id = jsonres["$respon_id"];
        }
        if(id==-1)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
      } catch (const t_json::exception &e) {
      }
    }
    //std::cout<<"ID RESPON: "<<id<<"\n"; 
    return_data d;
    d.callback = callback;
    d.respon_id = id;
    d.json_send = json;
    d.server_hash=server_hash;
    add_returns(d);
  }
  void send_response(t_json json_req,t_json json_res){
     
    int id = -1;
    std::string server_id;
    std::string ns=NAME_SERVER;
    t_json jdata;
    jdata["meta"]=json_req["meta"];
    jdata["data"]=json_res["data"];
    jdata["meta"]["$type_event"]="res";
    jdata["meta"]["$type_obj"]=json_res["meta"]["$type_obj"];
    std::reverse(jdata["meta"]["$list_servers"].begin(), jdata["meta"]["$list_servers"].end());
    
    while (id == -1) {
      try {
        t_json jsonres = cw.get_page_json(
            "/api/send/"+server_hash+"/"+ns+"/command/" + hash_id+"/event", jdata.dump());
        //std::cout<<"RES: "<<jsonres.dump()<<"\n";
        if(jsonres.contains("$error")){
          get_my_id();
        }else{
          id = jsonres["$respon_id"];
        }
        if(id==-1)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
      } catch (const t_json::exception &e) {
      }
    }
    std::cout<<"RESPON: "<<json_req["meta"]["$respon_id"].dump()<<"\n";
    end_event(json_req["id"]);
  }
  void add_handler(std::string nameobj,void (*callback)(connector_manager* conn,t_json json_req)){
    handler h;
    h.callback=callback;
    h.nameobj=nameobj;
    handlers.push_back(h);
  }
  t_json get_all_events(){
    return last_events;
  }
  int start_event(std::string event_id){
  int id = -1;
    std::string server_id;
    std::string ns=NAME_SERVER;
    
    while (id == -1) {
      try {
        t_json jsonres = cw.get_page_json(
            "/api/send/"+server_hash+"/"+ns+"/command/" + hash_id+"/event/start/"+event_id);
        //std::cout<<"RES: "<<jsonres.dump()<<"\n";
        if(jsonres.contains("$error")){
          get_my_id();
        }else{
          id = jsonres["$status"];
        }
        if(id==-1)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
      } catch (const t_json::exception &e) {
      }
    }
    return id;
  }
  int end_event(std::string event_id){
    int id = -1;
    std::string server_id;
    std::string ns=NAME_SERVER;
    
    while (id == -1) {
      try {
        t_json jsonres = cw.get_page_json(
            "/api/send/"+server_hash+"/"+ns+"/command/" + hash_id+"/event/finish/"+event_id);
        std::cout<<jsonres.dump()<<"\n";
        //std::cout<<"RES: "<<jsonres.dump()<<"\n";
        if(jsonres.contains("$error")){
          get_my_id();
        }else{
          id = jsonres["$status"];
        }
        if(id==-1)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
      } catch (const t_json::exception &e) {
      }
    }
    return id;
  }
  void getevent(){
    t_json jsonres;
    std::string ns=NAME_SERVER;
    while (jsonres.is_null()) {
      try {
        jsonres = cw.get_page_json(
            "/api/get/"+server_hash+"/"+ns+"/command/" + hash_id + "/event");
        
      } catch (const t_json::exception &e) {
      }
    }
    if(jsonres.contains("$error")){
      get_my_id();
     return;
    }
    if(jsonres.contains("events")){
        this->last_events=jsonres;
      for(int i=0;i<jsonres["events"].size();i++){
        
        
        if(jsonres["events"][i]["meta"]["$type_event"]=="res"){
        
          int index=search_returns(jsonres["events"][i]["meta"]["$respon_id"],jsonres["events"][i]["meta"]["$server_hash"]);
          if(index!=-1){
            if(start_event(jsonres["events"][i]["id"])!=-2){
              returns[index].callback(returns[index].json_send,jsonres["events"][i]);
              end_event(jsonres["events"][i]["id"]);
            }
            init_return_data(&returns[index]);
          } 
        }else if(jsonres["events"][i]["meta"]["$type_event"]=="req"){
          for(int j=0;j<handlers.size();j++){
            if(handlers[j].nameobj==jsonres["events"][i]["meta"]["$type_obj"]){
              if(start_event(jsonres["events"][i]["id"])!=-2){
                handlers[j].callback(this,jsonres["events"][i]);
                end_event(jsonres["events"][i]["id"]);
              }
              break;
            }
          }
        }
      }
    }
    std::cout<<"EVENT: "<<jsonres.dump()<<"\n";
  }
  void start_loop(){
    work_loop = true;
    th = new std::thread(&connector_manager::loop, this);
  }
  void loop() {
     
    
    while (work_loop == true) {
      getevent();
      sleep(1);
    }
    
  }
  void finish_loop() {
    work_loop = false;
    th->join();
    delete th;
  }
  ~connector_manager() {
    finish_loop();
    std::string code = "";
    std::string ns=NAME_SERVER;
    while (code == "") {
      try {
        code = cw.get_page("/api/client/"+server_hash+"/"+ns+"/command/" + hash_id+
                           "/exit");
      } catch (const t_json::exception &e) {
      }
    }
    
    std::cout << "CODE: " << code << "\n";
  }

private:
  std::thread* th;
  std::vector<return_data> returns;
  std::vector<handler> handlers;
  t_json last_events;
  
  curl_wrapper cw;

  bool work_loop=false;
};  