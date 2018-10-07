#include "uicontrol.h"
#include <iostream>


UIControl::UIControl(QObject *parent) : QObject(parent) {


}

void UIControl::ftp_connect() {
  asioex.ftp_connect();
}

void UIControl::OnClick2() {
//  asioex.sample2();
}
