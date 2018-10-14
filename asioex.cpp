#include "asioex.h"
#include "ftpclient.h"

Asioex::Asioex() {

}

void Asioex::ftp_connect() {

  std::thread t([]() {
    FTPClient ftp;/*("192.168.100.4", "21");*/
    ftp.Connect("192.168.100.4", "21", "upload", "user");
    auto buf = ftp.Download("/600.zip");
//    FILE* f = fopen("/Users/kmlee/hi.zip", "wb");
//    fwrite(&buf[0], 1, buf.size(), f);
//    auto buf = ftp.Download("/foo.xml");

    ftp.Upload(buf, "/abc/hihi.zip");

  });

  t.detach();
}
