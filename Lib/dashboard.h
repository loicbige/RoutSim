//
// Created by Bigé Loïc on 15/01/2026.
//

#ifndef ROUTSIM_DASHBOARD_H
#define ROUTSIM_DASHBOARD_H

#define DASHBOARD_PORT 8080
#define DASHBOARD_IP "192.168.1.182"
#define DASHBOARD_MESSAGE_LENGTH 128
#include "tabrout.h"
int sendHello(const int id, const char* ip);
int sendLinkUp(const int id1, const int id2);
int sendLinkDown(const int id1, const int id2);

#endif //ROUTSIM_DASHBOARD_H