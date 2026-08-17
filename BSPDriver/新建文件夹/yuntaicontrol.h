#pragma once


namespace YUNTAICONTROL
{
    class ytcontrol
    {
    public:
        bool Initialize_Port(const char* portName, int baudRate);
        void Close_Port();
        bool SendCommand(unsigned char* cmd, int length);
        bool Move_Vertical(int value);
        bool Move_Horizon(int value);
        bool Read_Vertical_Angle(unsigned char* buffer, int bufferSize, int* bytesRead, int timeoutMs);
        bool Read_Horizontal_Angle(unsigned char* buffer, int bufferSize, int* bytesRead, int timeoutMs);
        bool Open_Self_Check();
        bool Close_Self_Check();
        bool Return_Zero();
    private:

    };
}


