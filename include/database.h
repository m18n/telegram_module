#pragma once
#include<iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
struct mysql_conn_info {
    mysql_conn_info(){

    }
  mysql_conn_info(std::string ip, std::string name, std::string password,
                  std::string database, std::string port) {
    this->ip = ip;
    this->name = name;
    this->password = password;
    this->database = database;
    this->port = port;
  }
  std::string ip;
  std::string name;
  std::string password;
  std::string database;
  std::string port;
  void show() {
    std::cout << "ip: " << ip << " name:e " << name << " password: " << password
              << " Database: " << database << " PORT: " << port << "\n";
  }
};
class DB {

private:
  sql::Driver *driver;
  sql::Connection *con = NULL;
  sql::Statement *stmt;
  sql::ResultSet *res;
  bool isconn = false;
  mysql_conn_info last_info;

public:

  DB() {
    driver = get_driver_instance();
    // con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
    // con->setSchema("test");
  }
  bool isConnect() { return isconn; }
  bool connect(mysql_conn_info info) {
    last_info = info;
    try {
      con = driver->connect("tcp://" + info.ip + ":" + info.port, info.name,
                            info.password);
      con->setSchema(info.database);
      isconn = con->isValid();
      return isconn;
    } catch (const sql::SQLException &error) {
      std::cout << "ERROR MYSQL: " << error.what() << "\n";
      isconn = false;
      return isconn;
    }
  }
  ~DB() {
    if (con != NULL && con->isValid())
      delete con;
  }
};