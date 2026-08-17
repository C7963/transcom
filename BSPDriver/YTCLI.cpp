#include "YTCLI.h"


using namespace YTControlCLI;
using namespace YUNTAICONTROL;
using Marshal = Runtime::InteropServices::Marshal;

YTCli::YTCli()
{
    yt = new ytcontrol();
}

YTCli::~YTCli()
{
    delete yt;
}

bool YTCli::InitializePort(String^ portName, int baudRate)
{
    const char* native_name = (const char*)(Marshal::StringToHGlobalAnsi(portName).ToPointer());
    yt->Initialize_Port(native_name, baudRate);
    Marshal::FreeHGlobal(IntPtr((void*)native_name));
    return 0;
}

void YTCli::ClosePort()
{
    yt->Close_Port();
}

void YTCli::SetHorizonAngle(int angle)
{
    int current_angle = ReadHorizon();
    int res = current_angle + angle;
    yt->Move_Horizon(res);
}

void YTCli::SetVerticalAngle(int angle)
{
    int current_angle = ReadVertical();
    int res = current_angle - angle;
    yt->Move_Vertical(res);
}



//bool YTCli::ReadVerticalAngle([Out] array<unsigned char>^% buffer, [Out] int% bytesRead, int timeoutMs)
//{
//    const int bufferSize = 7;
//    unsigned char nativeBuffer[bufferSize];
//    int nativeBytesRead = 0;
//
//    bool success = yt->Read_Vertical_Angle(nativeBuffer, bufferSize, &nativeBytesRead, timeoutMs);
//
//    if (success && nativeBytesRead > 0)
//    {
//        buffer = gcnew array<unsigned char>(nativeBytesRead);
//        for (int i = 0; i < nativeBytesRead; i++)
//        {
//            buffer[i] = nativeBuffer[i];
//        }
//    }
//    else
//    {
//        buffer = gcnew array<unsigned char>(0);
//    }
//
//    bytesRead = nativeBytesRead;
//    return success;
//}
//
//bool YTCli::ReadHorizonAngle([Out] array<unsigned char>^% buffer, [Out] int% bytesRead, int timeoutMs)
//{
//    const int bufferSize = 7;
//    unsigned char nativeBuffer[bufferSize];
//    int nativeBytesRead = 0;
//
//    bool success = yt->Read_Horizontal_Angle(nativeBuffer, bufferSize, &nativeBytesRead, timeoutMs);
//
//    if (success && nativeBytesRead > 0)
//    {
//        buffer = gcnew array<unsigned char>(nativeBytesRead);
//        for (int i = 0; i < nativeBytesRead; i++)
//        {
//            buffer[i] = nativeBuffer[i];
//        }
//    }
//    else
//    {
//        buffer = gcnew array<unsigned char>(0);
//    }
//
//    bytesRead = nativeBytesRead;
//    return success;
//}

int YTCli::ReadHorizon()
{
    const int bufferSize = 7;
    unsigned char nativeBuffer[bufferSize];
    int nativeBytesRead = 0;
    int timeoutMs = 10;
    bool success = yt->Read_Horizontal_Angle(nativeBuffer, bufferSize, &nativeBytesRead, timeoutMs);
    unsigned int value = (nativeBuffer[4] << 8) | nativeBuffer[5];
    unsigned int HorizonAngle = (value + 50) / 100;

    return HorizonAngle;
}

int YTCli::ReadVertical()
{
    const int bufferSize = 7;
    unsigned char nativeBuffer[bufferSize];
    int nativeBytesRead = 0;
    int timeoutMs = 10;
    bool success = yt->Read_Vertical_Angle(nativeBuffer, bufferSize, &nativeBytesRead, timeoutMs);
    unsigned int value = (nativeBuffer[4] << 8) | nativeBuffer[5];
    unsigned int VerticalAngle = (value + 50) / 100;
    //VerticalAngle = 30- VerticalAngle;
    return VerticalAngle;
}

bool YTCli::ReturnZero()
{
    return yt->Return_Zero();
}