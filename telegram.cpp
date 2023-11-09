#include "telegram.h"

std::string TelegramClient::get_user_name(std::int64_t user_id) const {
  auto it = users_.find(user_id);
  if (it == users_.end()) {
    return "unknown user";
  }
  return it->second->first_name_ + " " + it->second->last_name_;
}
void getcode(t_json json_send,t_json json_answer){
  static int i=0;
  static auto start_time=std::chrono::high_resolution_clock::now();
  static auto end_time = std::chrono::high_resolution_clock::now();
  static auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
  if(i==0){
    start_time = std::chrono::high_resolution_clock::now();
    
  }else if(i==29){
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    std::cout<<"TIME: "<<duration.count()<<"\n";
  }
  std::cout<<"\n INDEX:"<<i<<" CODE: "<<json_answer["data"].dump()<<"\n\n";
  i++;
}
std::uint64_t TelegramClient::next_query_id() { return ++current_query_id_; }
void TelegramClient::send_query(td_api::object_ptr<td_api::Function> f,
                                std::function<void(Object)> handler) {
  auto query_id = next_query_id();
  if (handler) {
    handlers_.emplace(query_id, std::move(handler));
  }
  client_manager_->send(client_id_, query_id, std::move(f));
}
void TelegramClient::process_response(td::ClientManager::Response response) {
  if (!response.object) {
    return;
  }
  // std::cout << response.request_id << " " << to_string(response.object) <<
  // std::endl;
  if (response.request_id == 0) {
    return process_update(std::move(response.object));
  }
  auto it = handlers_.find(response.request_id);
  if (it != handlers_.end()) {
    it->second(std::move(response.object));
    handlers_.erase(it);
  }
}
auto TelegramClient::create_authentication_query_handler() {
  return [this, id = authentication_query_id_](Object object) {
    if (id == authentication_query_id_) {
      check_authentication_error(std::move(object));
    }
  };
}

void TelegramClient::on_authorization_state_update() {
  authentication_query_id_++;
  td_api::downcast_call(
      *authorization_state_,
      overloaded(
          [this](td_api::authorizationStateReady &) {
            are_authorized_ = true;
            std::cout << "Authorization is completed" << std::endl;
            AuthComplite();
          },
          
          [this](td_api::updateAuthorizationState &) {
            are_authorized_ = true;
            std::cout << "Authorization is completed" << std::endl;
            AuthComplite();
          },
          [this](td_api::authorizationStateLoggingOut &) {
            are_authorized_ = false;
            std::cout << "Logging out" << std::endl;
            AuthTerminate();
          },
          [this](td_api::authorizationStateClosing &) {
            std::cout << "Closing" << std::endl;
            AuthTerminate();
          },
          [this](td_api::authorizationStateClosed &) {
            are_authorized_ = false;
            need_restart_ = true;
            std::cout << "Terminated" << std::endl;
            AuthTerminate();
          },
          [this](td_api::authorizationStateWaitPhoneNumber &) {
            std::cout<<"PHONEE\n";
            AuthInputCode();
            send_query(
                td_api::make_object<td_api::setAuthenticationPhoneNumber>(
                    "+380970018947", nullptr),
                create_authentication_query_handler());
          },
          [this](td_api::authorizationStateWaitEmailAddress &) {
            std::cout << "Enter email address: " << std::flush;
            std::string email_address;
            std::cin >> email_address;
            send_query(
                td_api::make_object<td_api::setAuthenticationEmailAddress>(
                    email_address),
                create_authentication_query_handler());
          },
          [this](td_api::authorizationStateWaitEmailCode &) {
            std::cout << "Enter email authentication code: " << std::flush;
            std::string code;
            std::cin >> code;
            send_query(
                td_api::make_object<td_api::checkAuthenticationEmailCode>(
                    td_api::make_object<td_api::emailAddressAuthenticationCode>(
                        code)),
                create_authentication_query_handler());
          },
          [this](td_api::authorizationStateWaitCode &) {
           
            AuthInputCode();
            
          },
          [this](td_api::authorizationStateWaitRegistration &) {
            std::string first_name;
            std::string last_name;
            std::cout << "Enter your first name: " << std::flush;
            std::cin >> first_name;
            std::cout << "Enter your last name: " << std::flush;
            std::cin >> last_name;
            send_query(td_api::make_object<td_api::registerUser>(first_name,
                                                                 last_name),
                       create_authentication_query_handler());
          },
          [this](td_api::authorizationStateWaitPassword &) {
            std::cout << "Enter authentication password: " << std::flush;
            std::string password;
            std::getline(std::cin, password);
            send_query(td_api::make_object<td_api::checkAuthenticationPassword>(
                           password),
                       create_authentication_query_handler());
          },
          [this](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
            std::cout << "Confirm this login link on another device: "
                      << state.link_ << std::endl;
          },
          [this](td_api::authorizationStateWaitTdlibParameters &) {
            auto request = td_api::make_object<td_api::setTdlibParameters>();
            request->database_directory_ = "tdlib";
            request->use_message_database_ = true;
            request->use_secret_chats_ = true;
            request->api_id_ = 6329391;
            request->api_hash_ = "66791aca091c7a10cccf5a9998d80d0c";
            request->system_language_code_ = "en";
            request->device_model_ = "Desktop";
            request->application_version_ = "1.0";
            request->enable_storage_optimizer_ = true;
            send_query(std::move(request),
                       create_authentication_query_handler());
          }));
}
void TelegramClient::check_authentication_error(Object object) {
  if (object->get_id() == td_api::error::ID) {
    auto error = td::move_tl_object_as<td_api::error>(object);
    std::cout << "Error: " << to_string(error) << std::flush;
    on_authorization_state_update();
  }
}

void TelegramClient::process_update(td_api::object_ptr<td_api::Object> update) {
  td_api::downcast_call(
      *update,
      overloaded(
          [this](td_api::updateAuthorizationState &update_authorization_state) {
            authorization_state_ =
                std::move(update_authorization_state.authorization_state_);
            on_authorization_state_update();
          },
          [this](td_api::updateNewChat &update_new_chat) {

          },

          [this](td_api::updateUser &update_user) {
            auto user_id = update_user.user_->id_;
            users_[user_id] = std::move(update_user.user_);
          },
          [this](td_api::updateNewMessage &update_new_message) {
            auto chat_id = update_new_message.message_->chat_id_;
            std::string sender_name;
            td_api::downcast_call(
                *update_new_message.message_->sender_id_,
                overloaded(
                    [this, &sender_name](td_api::messageSenderUser &user) {
                      sender_name = get_user_name(user.user_id_);
                    },
                    [this, &sender_name](td_api::messageSenderChat &chat) {

                    }));
            std::string text;
            if (update_new_message.message_->content_->get_id() ==
                td_api::messageText::ID) {
              text = static_cast<td_api::messageText &>(
                         *update_new_message.message_->content_)
                         .text_->text_;
            }
            std::cout << "Receive message: [chat_id:" << chat_id
                      << "] [from:" << sender_name << "] [" << text << "]"
                      << std::endl;
          },
          [](auto &update) {}));
}