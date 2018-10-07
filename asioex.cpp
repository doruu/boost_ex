#include "asioex.h"

#include <cstddef>
#include <thread>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using boost::asio::ip::tcp;

namespace ftp {

bool command(tcp::socket& socket, std::string&& cmd) {
  try {
    boost::asio::streambuf b;
    std::ostream os(&b);
    os << (cmd+"\r\n");
    boost::asio::write(socket, b);
  } catch(std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  return true;
}

std::vector<char> read_data(tcp::socket& socket, size_t size) {
  std::cout << "read data" << std::endl;
  std::vector<char> buf;
  buf.resize(size);
  boost::asio::read(socket, boost::asio::buffer(buf));
  return buf;
}

std::string response(tcp::socket& socket) {
  boost::asio::streambuf res_buf;
  boost::asio::read_until(socket, res_buf, "\r\n");
  std::istream res_stream(&res_buf);
  std::string res;
  for (std::string line; std::getline(res_stream, line);) {
    res+=line;
  }
  return res;
}

size_t command_size(tcp::socket& socket, std::string path) {
  ftp::command(socket, std::string("SIZE ") + path);
  auto size_res = ftp::response(socket);
  std::vector<std::string> results;
  boost::split(results, size_res, [](char c){return c == ' ';});
  unsigned int code = std::stoi(results[0]);
  unsigned int size = std::stoi(results[1]);
  return size;
}

bool connect(boost::asio::io_service& io_service, tcp::socket& socket, std::string& host, std::string& port) {
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(host, port);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  tcp::resolver::iterator end;
  boost::system::error_code error = boost::asio::error::host_not_found;
  while (error && endpoint_iterator != end) {
    socket.close();
    socket.connect(*endpoint_iterator++, error);
  }
  if (!error) {
    cout << "socket connected: " << socket.is_open() << endl;
  }
  return true;
}

bool parse_pasv_response(std::string& res, std::string& server_ip, std::string& server_port) {
//  std::vector<std::string> results;
//  boost::split(results, res, [](char c){return c == ' ';});

//  size_t start = res.find('(')
//  size_t end = res.find(')')

  bool bret = true;
  unsigned int h[4];
  unsigned int p[2];
  char ip[100];
  size_t pos = res.find('(');
  std::string  str_ip = res.substr(pos + 1);
  std::sscanf(str_ip.c_str(), "%u,%u,%u,%u,%u,%u", &h[0], &h[1], &h[2], &h[3], &p[0], &p[1]);
  sprintf(ip, "%u.%u.%u.%u", h[0], h[1], h[2], h[3]);
  server_ip = ip;
  server_port = std::to_string(static_cast<unsigned short>(p[0] * 256 + p[1]));

  return bret;
}

}


class FtpClient {

public:
  FtpClient() = delete;
  FtpClient(std::string host, std::string port):host_(host), port_(port), socket_(io_service_), data_socket_(data_io_service_){}
  virtual ~FtpClient(){};

  bool Connect() {
    ftp::connect(io_service_, socket_, host_, port_);
    cout << ftp::response(socket_) << endl;
    return true;
  }
  bool Connect(std::string id, std::string pw) {
    bool bcon = ftp::connect(io_service_, socket_, host_, port_);
    cout << ftp::response(socket_) << endl;
    if (bcon) {
      ftp::command(socket_, std::string("USER ") + id);
      cout << ftp::response(socket_) << endl;
      ftp::command(socket_, std::string("PASS ") + pw);
      cout << ftp::response(socket_) << endl;
    }
    return bcon;
  }
  bool Download(std::string path) {

    size_t size = ftp::command_size(socket_, path);
    std::cout << "path size : " << size << std::endl;


    ftp::command(socket_, std::string("PASV"));
    auto pasv_res = ftp::response(socket_);
    std::cout << pasv_res << std::endl;
    ftp::parse_pasv_response(pasv_res, data_ip_, data_port_);


    ftp::command(socket_, std::string("RETR ") + path);
    std::cout << "send retr" << std::endl;
    ftp::connect(data_io_service_, data_socket_, data_ip_, data_port_);

    auto read = ftp::read_data(data_socket_, size);
    read.push_back('\0');
    std::string str(read.data());
    std::cout << read.size() << "  " << str << std::endl;

  }

private:
  std::string host_;
  std::string port_;

  boost::asio::io_service io_service_;
  tcp::socket socket_;

  boost::asio::io_service data_io_service_;
  tcp::socket data_socket_;

  std::string data_ip_;
  std::string data_port_;

};


Asioex::Asioex() {

}

void Asioex::ftp_connect() {

  std::thread t([]() {
    FtpClient ftp("ftp.dlptest.com", "21");
    ftp.Connect("dlpuser@dlptest.com", "e73jzTRTNqCN9PYAAjjn");
    ftp.Download("/FTP.txt");
  });

  t.detach();
}
