#pragma once
//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov
// (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
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
  std::string get_user_name(std::int64_t user_id) const;
  std::uint64_t next_query_id() ;
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

class TG : public TelegramClient {
public:
  TG() : TelegramClient() {
    
  }
  void Loop() {
    while (true) {
      process_response(client_manager_->receive(0));
    }
  }
  
private:
  std::vector<channel> channels;
  
private:
  int last_chat = 0;
  int total_chat = 0;

private:
  virtual void AuthComplite() { GetAllChannels(); }
  void CheckDBChannels(){

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
