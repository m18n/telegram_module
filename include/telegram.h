#pragma once

//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov
// (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "connector_manager.h"
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <vector>
// Simple single-threaded example of TDLib usage.
// Real world programs should use separate thread for the user input.
// Example includes user authentication, receiving updates, getting chat list
// and sending text messages.

// overloaded

namespace detail {
template <class... Fs> struct overload;

template <class F> struct overload<F> : public F {
  explicit overload(F f) : F(f) {}
};
template <class F, class... Fs>
struct overload<F, Fs...> : public overload<F>, public overload<Fs...> {
  overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {}
  using overload<F>::operator();
  using overload<Fs...>::operator();
};
} // namespace detail

template <class... F> auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}

namespace td_api = td::td_api;

class TelegramClient {
protected:
  using Object = td_api::object_ptr<td_api::Object>;
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_{0};
  td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
  bool are_authorized_{false};
  bool need_restart_{false};
  std::uint64_t current_query_id_{0};
  std::uint64_t authentication_query_id_{0};
  std::map<std::uint64_t, std::function<void(Object)>> handlers_;
  std::map<std::int64_t, td_api::object_ptr<td_api::user>> users_;

public:
  TelegramClient() {
    td::ClientManager::execute(
        td_api::make_object<td_api::setLogVerbosityLevel>(1));
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
    send_query(td_api::make_object<td_api::getOption>("version"), {});
    process_response(client_manager_->receive(0));
  }
  ~TelegramClient() {}

protected:
  virtual void AuthComplite() = 0;
  virtual void AuthInputCode() = 0;
  virtual void AuthTerminate() = 0;
  std::string get_user_name(std::int64_t user_id) const;
  std::uint64_t next_query_id();
  void send_query(td_api::object_ptr<td_api::Function> f,
                  std::function<void(Object)> handler);
  void process_response(td::ClientManager::Response response);
  auto create_authentication_query_handler();

  void on_authorization_state_update();
  void check_authentication_error(Object object);
  void process_update(td_api::object_ptr<td_api::Object> update);
};
struct channel {
  channel(int64_t id, std::string name) {
    this->id = id;
    this->name = name;
  }
  int64_t id;
  std::string name;
};
void getcode(t_json json_send,t_json json_answer);
class TG : public TelegramClient {
public:
  TG() : TelegramClient() {
    conn_m.name_client="telegram";
    conn_m.add_connection("127.0.0.1:3000");
    //conn_m.add_connection("127.0.0.1:3001");
    conn_m.on();
    //my_id=conn_m.get_my_id();
    t_json json;
    json["meta"]["$type_event"]="req";
    json["meta"]["$type_obj"]="code";
    json["meta"]["$list_servers"][0]={{"name","telegram"}};
    json["meta"]["$list_servers"][1]={{"name","tasker"}};
    json["meta"]["$list_servers"][2]={{"name","global_tasker_node"}};
    json["meta"]["$list_servers"][3]={{"name","web"}};
    json["data"]={{"test","\n%$sd\"}f&"}};
    std::cout<<"SERVERS: " <<json["meta"]["$list_servers"].dump()<<"\n";
    //conn_m.send(json,getcode);  
    for(int i=0;i<1;i++){
    conn_m.send("127.0.0.1:3000",json,getcode);
    //conn_m.send("127.0.0.1:3001",json,getcode);
    }
    
    std::cout<<"MY ID: "<<my_id<<"\n";
  }
  void Loop() {
    
    while (true) {
      process_response(client_manager_->receive(0));
    }
  }
  void showchannels() {
    for (int i = 0; i < channels.size(); i++) {
      std::cout << "CHANNELS ID: " << channels[i].id
                << " NAME: " << channels[i].name << "\n";
    }
  }

private:
  std::vector<channel> channels;
  connector::connector_manager conn_m;
  int my_id=0;
private:
  int last_chat = 0;
  int total_chat = 0;

private:
  virtual void AuthComplite() { GetAllChannels(); }
  virtual void AuthInputCode() {
    std::string code="";
    //code = conn_m.get_auth_code();
    
    // send_query(
    //             td_api::make_object<td_api::checkAuthenticationCode>(code),
    //             [this, id = authentication_query_id_](Object object) {
    //   if (id == authentication_query_id_) {
    //     check_authentication_error(std::move(object));
    //   }});
  }
  virtual void AuthTerminate(){
    std::string code="";
    //code = conn_m.get_auth_code();
    
  }
  void CheckDBChannels(){
      showchannels();
  }
  void GetAllChannels() {
      auto chats = td_api::make_object<td_api::getChats>(nullptr, 100);

      send_query(std::move(chats), [this](Object object) {
        if (object->get_id() == td_api::error::ID) {
          return;
        }

        auto chats = td::move_tl_object_as<td_api::chats>(object);

        // if(id==td::td_api::chatTypeSupergroup::ID){
        this->last_chat = 0;
        this->total_chat = chats->total_count_;

        for (int i = 0; i < chats->total_count_; i++) {
          auto chat = td_api::make_object<td_api::getChat>();
          chat->chat_id_ = chats->chat_ids_[i];
          send_query(std::move(chat), [this](Object object) {
            if (object->get_id() == td_api::error::ID) {
              return;
            }

            auto chat = td::move_tl_object_as<td_api::chat>(object);
            if (chat->type_->get_id() == td::td_api::chatTypeSupergroup::ID) {

              channels.push_back(channel(chat->id_, chat->title_));
              // this->chats[chat->id_]=chat->title_;
            }
            this->last_chat++;
            if (last_chat == this->total_chat) {
              this->CheckDBChannels();
            }
          });
        }
      });
  }
  };
