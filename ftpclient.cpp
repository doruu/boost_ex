#include "ftpclient.h"

FTPClient::FTPClient() : socket_(io_service_), data_socket_(data_io_service_){

}

bool FTPClient::Connect(std::string host, std::string port, std::string id, std::string pw) {

  if (!Connect(socket_, io_service_, host, port) ) {
    BOOST_LOG_TRIVIAL(debug) << "Connect Failed";
    return false;
  } else {
    auto res = Response(socket_);
    BOOST_LOG_TRIVIAL(debug) << res;
  }

  if (!CommandLogin(socket_, id, pw)) {
    return false;
  }
  return true;
}

auto FTPClient::Download(std::string path)
  -> std::vector<char> {
  std::string data_ip;
  std::string data_port;
  CommandPassive(socket_, data_ip, data_port);
  Connect(data_socket_, data_io_service_, data_ip, data_port);
  size_t size = CommandSize(socket_, path);
  CommandRETR(socket_, path);
  auto ret = ReadData(data_socket_,  size);
  data_socket_.close();
  auto res = Response(socket_);
  BOOST_LOG_TRIVIAL(debug) << "Downlaod Finish res: " << res;
  return ret;
}

bool FTPClient::Upload(std::vector<char>& buf, std::string path) {
  std::string data_ip;
  std::string data_port;
  CommandPassive(socket_, data_ip, data_port);
  Connect(data_socket_, data_io_service_, data_ip, data_port);
  CommandSTOR(socket_, path);
  try {
    BOOST_LOG_TRIVIAL(debug) << "Upload Data size " << buf.size();
    boost::asio::write(data_socket_,
                       boost::asio::buffer(buf, buf.size()),
                       boost::asio::transfer_all());


    data_socket_.close();
    auto res = Response(socket_);
    BOOST_LOG_TRIVIAL(debug) << "Upload Finish res: " << res;
  } catch(std::exception& e) {
    BOOST_LOG_TRIVIAL(debug) << e.what();
  }
  return true;
}

bool FTPClient::Connect(tcp::socket& socket, boost::asio::io_service& io_service,
                        std::string& host, std::string& port) {
  BOOST_LOG_TRIVIAL(debug) << "Connect " << host << ":" << port;
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
    BOOST_LOG_TRIVIAL(debug) << "Socket Connected " << socket.is_open();
  } else {
    BOOST_LOG_TRIVIAL(debug) << error;
  }
  return true;
}

bool FTPClient::Command(tcp::socket& socket, std::string& cmd) {
  try {
    boost::asio::streambuf b;
    std::ostream os(&b);
    os << (cmd + "\r\n");
    boost::asio::write(socket, b);
  } catch(std::exception& e) {
    BOOST_LOG_TRIVIAL(debug) << e.what();
  }
  return true;
}

auto FTPClient::ReadData(tcp::socket& socket, size_t size)
  -> std::vector<char> {
  std::vector<char> buf;
  buf.resize(size);
  boost::system::error_code error;
  std::size_t n = boost::asio::read(
      socket, boost::asio::buffer(buf),
      boost::asio::transfer_all(), error);
  if (error) {
    BOOST_LOG_TRIVIAL(debug) << "Error  " << error << " Bytes";
    throw boost::system::system_error(error);
  }
  BOOST_LOG_TRIVIAL(debug) << "Read " << size << " Bytes";
  return buf;
}

std::string FTPClient::Response(tcp::socket& socket) {
  boost::asio::streambuf res_buf;
  boost::asio::read_until(socket, res_buf, "\r\n");
  std::istream res_stream(&res_buf);
  std::string res;
  std::string line;
  for (std::string line; std::getline(res_stream, line);) {
    res+=line;
  }
  return res;
}

size_t FTPClient::CommandSize(tcp::socket& socket, std::string path) {
  std::string cmd = std::string("SIZE ") + path;
  Command(socket, cmd);
  auto size_res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "SIZE res : " << size_res;
  std::vector<std::string> results;
  boost::split(results, size_res, [](char c){return c == ' ';});
  int code = std::stoi(results[0]);
  size_t size = static_cast<size_t>(std::stoi(results[1]));
  return size;
}

bool FTPClient::CommandLogin(tcp::socket& socket, std::string id, std::string pw) {
  std::string cmd = std::string("USER ") + id;
  Command(socket, cmd);
  auto res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "USER res : " << res;

  cmd = std::string("PASS ") + pw;
  Command(socket, cmd);
  res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "PASS res : " << res;
  return true;
}

bool FTPClient::CommandRETR(tcp::socket& socket, std::string path) {
  std::string cmd = std::string("RETR ") + path;
  Command(socket, cmd);
  auto res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "RETR res : " << res;
  return true;
}

bool FTPClient::CommandREST(tcp::socket& socket) {
  std::string cmd = std::string("REST");
  Command(socket, cmd);
  auto res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "REST res : " << res;
  return true;
}

bool FTPClient::CommandSTOR(tcp::socket& socket, std::string path) {
  std::string cmd = std::string("STOR ") + path;
  Command(socket, cmd);
  auto res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "STOR res:" << res;
  return true;
}

bool FTPClient::CommandPassive(tcp::socket& socket, std::string& data_ip, std::string& data_port) {
  std::string cmd = "TYPE LOCAL";
  Command(socket, cmd);
  auto res = Response(socket);
  BOOST_LOG_TRIVIAL(debug) << "Type res:" << res;

  cmd = "PASV";
  Command(socket, cmd);
  res = Response(socket);
  BOOST_LOG_TRIVIAL(debug)  << "PASV res:" << res;

  bool b = ParsePassiveResponse(res, data_ip, data_port);
  return b;
}

bool FTPClient::ParsePassiveResponse(std::string& res, std::string& data_ip, std::string& data_port) {
  size_t start = res.find('(');
  size_t end = res.find(')');
  if (start == std::string::npos || end == std::string::npos) {
    return false;
  }
  std::string data = res.substr(start + 1, end - start - 1);
  BOOST_LOG_TRIVIAL(debug) << data;
  std::vector<std::string> r;
  boost::split(r, data, [](char c){return c == ',';});
  if (r.size() != 6) {
    return false;
  }
  // try catch stoi
  data_ip = r[0] + "." + r[1] + "." + r[2] + "." + r[3];
  int p1 = std::stoi(r[4]);
  int p2 = std::stoi(r[5]);
  data_port = std::to_string(static_cast<unsigned short>(p1 * 256 + p2));
  return true;
}
