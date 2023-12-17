#ifndef WLANNET_H
#define WLANNET_H

#include <QString>

typedef struct _ApInfo {
    QString wifi_name;
    QString wifi_pw;
    bool is_enable;
} ApInfo;

unsigned long SettingHostpot(const QString &name, const QString &pw);

unsigned long HostpotStart();

unsigned long HostpotStop();

#endif // WLANNET_H
