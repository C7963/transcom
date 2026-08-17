#pragma once  
#include "yuntaicontrol.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace YTControlCLI {

    public ref class YTCli {
    public:
        YTCli();
        ~YTCli();
        bool InitializePort(String^ portName, int baudRate);
        void ClosePort();
        //bool SendCommand(array<unsigned char>^ cmd, int length);

        void SetHorizonAngle(int angle);
        void SetVerticalAngle(int angle);
        //bool ReadVerticalAngle([Out] array<unsigned char>^% buffer, [Out] int% bytesRead, int timeoutMs);
        //bool ReadHorizonAngle([Out] array<unsigned char>^% buffer, [Out] int% bytesRead, int timeoutMs);
        int ReadHorizon();
        int ReadVertical();
        bool ReturnZero();


       
    private:
      
        YUNTAICONTROL::ytcontrol* yt;

    };
}