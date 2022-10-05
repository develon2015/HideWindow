#include <iostream>
#include <fstream>
#include <string>
#include "include.h"

using namespace std;

void handle_hosts()
{
    const auto path = R"(C:\Windows\System32\drivers\etc\hosts)";
    // Google update service API in CHN: update.googleapis.com
    const auto googleapi = R"(180.163.151.32 translate.googleapis.com)";

    char cmd[64] = "explorer ";
    strcat(cmd, path);
    WinExec(cmd, SW_HIDE);

    init_float_window();
    string line;
    ifstream hosts(path);
    if (hosts.is_open())
    {
        while (getline(hosts, line))
        {
            if (line.compare(googleapi) == 0) {
                hosts.close();
                toast(L"hosts 中已有谷歌翻译 translate.googleapis.com");
                return;
            }
        }
        hosts.close();
    } else {
        toast(L"无法读取 hosts 文件");
        return;
    }

    ofstream whosts(path, ios_base::app);
    if (whosts.is_open())
    {
        whosts << endl << googleapi << endl;
        whosts.close();
        toast(L"已在 hosts 中添加谷歌翻译 translate.googleapis.com");
    } else {
        toast(L"无法写入 hosts 文件");
        return;
    }
}
