#pragma once
#include <string>
#include <vector>
#include "XillyFile.h"

namespace Device
{
#define APIOUT __declspec (dllexport)

	using namespace std;

	/// <summary>
	/// Memory32 Device操作类
	/// </summary>
	class APIOUT NVMe_Write
	{
	private:
		~NVMe_Write();
		NVMe_Write();
		static NVMe_Write* instance;
		bool privateEOF = false;
		string filePath;
		CommBus::XillyFile* device;
	public:
		static NVMe_Write* getInstance() {
			if (instance == nullptr)
			{
				instance = new NVMe_Write();
			}
			return instance;
		};
		vector<unsigned char> data; //读取的数据缓存

		bool getEOF() const;

		void setEOF(const bool& value);

		/// <summary>
		/// 设置int类型命令参数
		/// </summary>
		/// <param name="address"></param>
		/// <param name="cmd"></param>
		/// <param name="needKeepFileOpen"></param>
		/// <returns></returns>
		bool SendData(unsigned char* cmd, bool needKeepFileOpen = false);
	};
}
