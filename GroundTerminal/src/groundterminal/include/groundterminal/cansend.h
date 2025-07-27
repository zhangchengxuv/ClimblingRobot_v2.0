#ifndef CANSEND_H
#define CANSEND_H

#include <iostream>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <net/if.h>
#include <cstdlib>

#define BYTE unsigned char

uint8_t receiveData[4];
uint8_t realData[2];
uint8_t positionData[4];
int action_flag;

void *receive_func(void *param);


// 发送CAN帧的函数
// can卡序号 canid 是由扩展帧 数据 数据长度
void SendData(int sock, uint32_t id, bool isExtended, const uint8_t *data, int dataLen)
{
    if (sock < 0)
    {
        std::cerr << "CAN未初始化！" << std::endl;
        return;
    }

    struct can_frame frame;

    // 设置CAN帧的ID
    if (isExtended)
    {
        frame.can_id = id | CAN_EFF_FLAG; // 设置扩展帧标志
    }
    else
    {
        frame.can_id = id; // 标准帧
    }

    // 设置数据长度
    frame.can_dlc = dataLen;

    // 复制数据到CAN帧的数据字段
    for (int i = 0; i < dataLen; i++)
    {
        frame.data[i] = data[i];
    }

    // 发送CAN帧
    if (write(sock, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
    {
        std::cerr << "发送CAN帧失败！" << std::endl;
    }
    else
    {       
    }
}

// 初始化CAN接口
int init_can(const char *ifname)
{
    // 配置和启动CAN接口
    // 定义了两个字符数组
    char commde_cmd[100];
    char up_cmd[100];
    // 注意管理员密码和波特率
    snprintf(commde_cmd, sizeof(commde_cmd), "echo 123456 | sudo -S ip link set %s type can bitrate 1000000", ifname);
    snprintf(up_cmd, sizeof(up_cmd), "echo 123456 | sudo ip link set up %s", ifname);
    system(commde_cmd);
    system(up_cmd);

    // 创建socket
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0)
    {
        std::cerr << "创建socket失败！" << std::endl;
        return -1;
    }

    // 设置CAN接口名称
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;

    // 获取CAN接口的索引
    int ifindex = if_nametoindex(ifname);
    if (ifindex == 0)
    {
        std::cerr << "获取CAN接口索引失败！" << std::endl;
        close(sock);
        return -1;
    }
    addr.can_ifindex = ifindex;

    // 绑定套接字到CAN接口
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "绑定到CAN接口失败！" << std::endl;
        close(sock);
        return -1;
    }

    return sock;
}

#endif