#include "yuntaicontrol.h"
#include <windows.h>
#include <string>
#include <sstream>

using namespace YUNTAICONTROL;

// 全局变量
HANDLE hSerial = INVALID_HANDLE_VALUE;
std::string lastError;

// 初始化串口
bool ytcontrol::Initialize_Port(const char* portName, int baudRate) {
    // 如果已经打开，先关闭
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }

    // 打开串口
    hSerial = CreateFileA(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        lastError = "无法打开串口: " + std::string(portName);
        return false;
    }

    // 配置串口参数
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        lastError = "获取串口状态失败";
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return false;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        lastError = "设置串口参数失败";
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return false;
    }

    // 设置超时
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        lastError = "设置串口超时失败";
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return false;
    }

    lastError = "串口初始化成功";
    return true;
}


void ytcontrol::Close_Port() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
    lastError = "串口已关闭";
}


bool ytcontrol::SendCommand(unsigned char* cmd, int length) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        lastError = "串口未打开";
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hSerial, cmd, length, &bytesWritten, NULL)) {
        lastError = "发送指令失败";
        return false;
    }

    if (bytesWritten != length) {
        lastError = "指令发送不完整";
        return false;
    }

    std::stringstream ss;
    ss << "发送指令: ";
    for (int i = 0; i < length; i++) {
        if (i > 0) ss << "-";
        char buf[3];
        sprintf_s(buf, sizeof(buf), "%02X", cmd[i]);
        ss << buf;
    }
    lastError = ss.str();

    return true;
}


bool ytcontrol::Move_Vertical(int value) {
    int res = value * 100;
    unsigned short res16 = (unsigned short)(res & 0xFFFF);
    unsigned char highByte = (unsigned char)(res16 >> 8);
    unsigned char lowByte = (unsigned char)(res16 & 0xFF);

    unsigned char cmd[7];
    unsigned char checksum = 0;

    cmd[0] = 0xFF;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = 0x4D;
    cmd[4] = highByte;
    cmd[5] = lowByte;

    for (int i = 1; i <= 5; i++) {
        checksum += cmd[i];
    }
    cmd[6] = checksum;

    return SendCommand(cmd, 7);
}


bool ytcontrol::Move_Horizon(int value) {

    int res = value * 100;
    unsigned short res16 = (unsigned short)(res & 0xFFFF);
    unsigned char highByte = (unsigned char)(res16 >> 8);
    unsigned char lowByte = (unsigned char)(res16 & 0xFF);

    unsigned char cmd[7];
    unsigned char checksum = 0;

    cmd[0] = 0xFF;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = 0x4B;
    cmd[4] = highByte;
    cmd[5] = lowByte;

    for (int i = 1; i <= 5; i++) {
        checksum += cmd[i];
    }
    cmd[6] = checksum;

    return SendCommand(cmd, 7);
}


// 读取垂直角度数据的函数
bool ytcontrol::Read_Vertical_Angle(unsigned char* buffer, int bufferSize, int* bytesRead, int timeoutMs) {
    unsigned char cmd[7];
    cmd[0] = 0xFF;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = 0x53;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x54;
    SendCommand(cmd, 7);

    if (hSerial == INVALID_HANDLE_VALUE) {
        lastError = "串口未打开";
        return false;
    }

    if (buffer == nullptr || bufferSize <= 0) {
        lastError = "缓冲区无效";
        return false;
    }

    // 设置读取超时
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = timeoutMs;
    timeouts.ReadTotalTimeoutConstant = timeoutMs;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        lastError = "设置读取超时失败";
        return false;
    }

    DWORD dwBytesRead = 0;
    if (!ReadFile(hSerial, buffer, bufferSize, &dwBytesRead, NULL)) {
        lastError = "读取串口数据失败";
        return false;
    }

    if (bytesRead != nullptr) {
        *bytesRead = dwBytesRead;
    }

    // 记录读取到的数据
    if (dwBytesRead > 0) {
        std::stringstream ss;
        ss << "读取到 " << dwBytesRead << " 字节: ";
        for (DWORD i = 0; i < dwBytesRead; i++) {
            if (i > 0) ss << "-";
            char buf[3];
            sprintf_s(buf, sizeof(buf), "%02X", buffer[i]);
            ss << buf;
        }
        lastError = ss.str();
    }
    else {
        lastError = "未读取到数据（超时或设备无响应）";
    }

    return (dwBytesRead > 0);
}


bool ytcontrol::Read_Horizontal_Angle(unsigned char* buffer, int bufferSize, int* bytesRead, int timeoutMs)
{
    unsigned char cmd[7];
    cmd[0] = 0xFF;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = 0x51;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x52;
    SendCommand(cmd, 7);

    if (hSerial == INVALID_HANDLE_VALUE) {
        lastError = "串口未打开";
        return false;
    }

    if (buffer == nullptr || bufferSize <= 0) {
        lastError = "缓冲区无效";
        return false;
    }

    // 设置读取超时
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = timeoutMs;
    timeouts.ReadTotalTimeoutConstant = timeoutMs;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        lastError = "设置读取超时失败";
        return false;
    }

    DWORD dwBytesRead = 0;
    if (!ReadFile(hSerial, buffer, bufferSize, &dwBytesRead, NULL)) {
        lastError = "读取串口数据失败";
        return false;
    }

    if (bytesRead != nullptr) {
        *bytesRead = dwBytesRead;
    }

    // 记录读取到的数据
    if (dwBytesRead > 0) {
        std::stringstream ss;
        ss << "读取到 " << dwBytesRead << " 字节: ";
        for (DWORD i = 0; i < dwBytesRead; i++) {
            if (i > 0) ss << "-";
            char buf[3];
            sprintf_s(buf, sizeof(buf), "%02X", buffer[i]);
            ss << buf;
        }
        lastError = ss.str();
    }
    else {
        lastError = "未读取到数据（超时或设备无响应）";
    }

    return (dwBytesRead > 0);
}

bool ytcontrol::Open_Self_Check()
{
    unsigned char cmd[7] = { 0xFF, 0x01, 0x00, 0x07, 0x00, 0x69, 0x71 };
    return SendCommand(cmd, 7);
}

bool ytcontrol::Close_Self_Check()
{
    unsigned char cmd[7] = { 0xFF, 0x01, 0x00, 0x03, 0x00, 0x69, 0x6D };
    return SendCommand(cmd, 7);
}

bool ytcontrol::Return_Zero()
{
    unsigned char cmd[7] = { 0xFF, 0x01, 0x00, 0x4B, 0x44, 0x5C, 0xEC };
    return SendCommand(cmd, 7);
}