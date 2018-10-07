#ifndef UICONTROL_H
#define UICONTROL_H

#include <QObject>
#include "asioex.h"

class UIControl : public QObject
{
  Q_OBJECT
public:
  explicit UIControl(QObject *parent = nullptr);

  Q_INVOKABLE void ftp_connect();
  Q_INVOKABLE void OnClick2();

signals:

public slots:


private:
  Asioex asioex;
};

#endif // UICONTROL_H
