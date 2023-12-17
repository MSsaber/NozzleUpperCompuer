#pragma execution_character_set("utf-8")
#include <Windows.h>
#include <Wlanapi.h>
#include <QDebug>
#include "wlannet.h"

static ApInfo global_ap_info;;

unsigned long SettingHostpot(const QString &name, const QString &pw)
{
    DWORD ret = ERROR_SUCCESS;
    DWORD version;
    HANDLE client_handle = NULL;
    WLAN_HOSTED_NETWORK_REASON reason;
    WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS network_connect_setting;
    DOT11_SSID ssid;
    char *wifi_name = NULL;
    char *tmp_wifi_pw = NULL;
    char wifi_pw[32] = {0};

    if (global_ap_info.wifi_name == name &&
        global_ap_info.wifi_pw == pw) {
        //已存在的热点配置无需再次配置
        qDebug() << "Hostpot was setting " << ret;
        goto error;
    }

    wifi_name = _strdup(name.toLatin1());
    tmp_wifi_pw = _strdup(pw.toLatin1());
    ssid.uSSIDLength = name.length();
    memcpy(ssid.ucSSID, wifi_name, ssid.uSSIDLength);
    memcpy(wifi_pw, tmp_wifi_pw, pw.length());
    /*
     * dwClientVersion: 1 具有 SP3 的 Windows XP 的客户端版本和具有 SP2 的 Windows XP 的无线 LAN API
     *                  2 适用于 Windows Vista 和 Windows Server 2008 的客户端版本
    */
    ret = WlanOpenHandle(1, NULL, &version, &client_handle);
    qDebug() << "NegotiatedVersion :" << version;

    if (ret != ERROR_SUCCESS) {
        qDebug() << "Wlan open failed with " << ret;
        client_handle = NULL;
        goto error;
    } else {
        qDebug() << "Wlan handle open success";
    }

    /*
     * 初始化网络配置
    */
    ret = WlanHostedNetworkInitSettings(client_handle, &reason, NULL);
    if (ret != ERROR_SUCCESS) {
        qDebug() << "NetWork init failed with " << ret << ", reasion :" << reason;
        client_handle = NULL;
        goto error;
    } else {
        qDebug() << "NetWork init success";
    }

    /*
     * 设置无线托管网络的静态属性
    */
    network_connect_setting.hostedNetworkSSID = ssid;
    network_connect_setting.dwMaxNumberOfPeers = 8; //连接设备上限8个
    ret = WlanHostedNetworkSetProperty(client_handle,
                                       wlan_hosted_network_opcode_connection_settings,
                                       sizeof(network_connect_setting), &network_connect_setting, &reason, NULL);
    if (ret != ERROR_SUCCESS) {
        qDebug() << "NetWork set connect proerty failed with " << ret << ", reasion :" << reason;
        client_handle = NULL;
        goto error;
    } else {
        qDebug() << "NetWork proerty set success";
    }

    /*
     * 配置热点密码
    */
    qDebug() << "[" << pw << "]";
    qDebug() << "NetWork set key : [" << wifi_pw << "]," << strlen((char*)(wifi_pw));
    ret = WlanHostedNetworkSetSecondaryKey(client_handle, (DWORD)strlen((char*)(wifi_pw)) + 1,   //(这里的长度规定把'\0'算在内)
        (PUCHAR)(wifi_pw), TRUE, TRUE, &reason, NULL);
    if (ret != ERROR_SUCCESS) {
        qDebug() << "NetWork set key failed with " << ret << ", reasion :" << reason;
        client_handle = NULL;
        goto error;
    } else {
        qDebug() << "NetWork set key success";
    }

error:
    WlanCloseHandle(client_handle, NULL);
    return ret;
}

unsigned long HostpotStart()
{
    HANDLE client_handle = NULL;
    DWORD version;
    DWORD ret;
    BOOL enabled = TRUE;
    WLAN_HOSTED_NETWORK_REASON reason;

    if ((ret = WlanOpenHandle(1, NULL, &version, &client_handle)) != ERROR_SUCCESS) {
        qDebug() << "Wlan open failed with " << ret;
        goto error;
    }

    global_ap_info.is_enable = false;
    //如果承载网络被禁用（即netsh wlan set hostednetwork mode=disallow的作用），则先解除禁用状态。
    if (!global_ap_info.is_enable) {
        ret = WlanHostedNetworkSetProperty(
            client_handle,
            wlan_hosted_network_opcode_enable,
            sizeof(enabled),
            (PVOID)&enabled,
            &reason,
            NULL
        );
        if (ret != ERROR_SUCCESS) {
            qDebug() << "NetWork enable failed with " << ret << ", reasion :" << reason;
            goto error;
        } else {
            qDebug() << "NetWork enable";
        }
    }

    //启用承载网络。使用WlanHostedNetworkForceStart，即使本程序退出，承载网络仍会继续运行。
    if ((ret = WlanHostedNetworkForceStart(client_handle, &reason, NULL)) != ERROR_SUCCESS) {
        qDebug() << "NetWork start failed with " << ret << ", reasion :" << reason;
        goto error;
    }

error:
    WlanCloseHandle(client_handle, NULL);
    return ret;
}

unsigned long HostpotStop()
{
    HANDLE client_handle;
    DWORD version;
    DWORD ret;

    if ((ret = WlanOpenHandle(1, NULL, &version, &client_handle)) != ERROR_SUCCESS) {
        qDebug() << "Wlan open failed with " << ret;
        goto error;
    }

    WLAN_HOSTED_NETWORK_REASON reason;
    if ((ret = WlanHostedNetworkForceStop(client_handle, &reason, NULL)) != ERROR_SUCCESS) {
        qDebug() << "NetWork stop failed with " << ret << ", reasion :" << reason;
        goto error;
    }

error:
    WlanCloseHandle(client_handle, NULL);
    return ret;
}
