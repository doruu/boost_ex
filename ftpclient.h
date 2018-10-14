#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <boost/log/trivial.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace std;
using boost::asio::ip::tcp;


class FTPClient
{
public:

  FTPClient();

  bool Connect(std::string host, std::string port, std::string id, std::string pw);
  auto Download(std::string path)
    -> std::vector<char>;
  bool Upload(std::vector<char>& buf, std::string path);

private:

  boost::asio::io_service io_service_;
  boost::asio::io_service data_io_service_;

  tcp::socket socket_;
  tcp::socket data_socket_;


public:

  bool Connect(tcp::socket& socket, boost::asio::io_service& io_service,
               std::string& host, std::string& port);

  bool Command(tcp::socket& socket, std::string& cmd);

  size_t CommandSize(tcp::socket& socket, std::string path);

  bool CommandRETR(tcp::socket& socket, std::string path);

  bool CommandREST(tcp::socket& socket);

  bool CommandSTOR(tcp::socket& socket, std::string path);

  bool CommandLogin(tcp::socket& socket, std::string id, std::string pw);

  bool CommandPassive(tcp::socket& socket, std::string& data_ip, std::string& data_port);

  bool ParsePassiveResponse(std::string& res, std::string& server_ip, std::string& server_port);

  auto Response(tcp::socket& socket)
    -> std::string ;

  auto ReadData(tcp::socket& socket, size_t size)
    -> std::vector<char>;
};

#endif // FTPCLIENT_H
