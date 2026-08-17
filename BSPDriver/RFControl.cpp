#include <cstdint>
#include "RFControl.h"   
#include "Global.h"
using namespace RFCONTROL;
using namespace RPU_44;
using namespace HLJS386;
using namespace CM18_500M;
using namespace RF12;

#pragma region RFModuleDefinition
// ������Խӿ�?
class IRFModule {
public:
	virtual ~IRFModule() {}
	virtual bool SetCenterFreq(uint64_t freq) = 0;
	virtual RefLevelResult SetRefLevel(int level) = 0;
	virtual void SetPowerOnOff(uint32_t flag) { /* Ĭ�Ͽ�ʵ�� */ }
	virtual void SetOutBW(int bw) { /* Ĭ�Ͽ�ʵ�� */ }
	virtual void SetIFATT(uint32_t ifatt) { /* Ĭ�Ͽ�ʵ�� */ }
	virtual uint32_t  GetIFATT() = 0 { /* Ĭ�Ͽ�ʵ�� */ }
	virtual uint32_t  GetRFATT() = 0 { /* Ĭ�Ͽ�ʵ�� */ }
};

// RF12 ģ�����?
class RF12Module : public IRFModule {
	RF_12* impl;
public:
	RF12Module() {
		impl = new RF_12(); 
		impl->rf_12_init();
	}
	~RF12Module() override { delete impl; }
	bool SetCenterFreq(uint64_t freq) override { return impl->SetCenterFreq(freq); }
	RefLevelResult SetRefLevel(int level) override {
		RefLevelResult result = { 0, 0 };
		impl->SetRefLevel(level);
		result.Att = level;

		// ���������߼�
		if (level >= 0 && level <= 30) {
			result.FFTGainOffset = 0;
		}
		else if (level >= -20 && level < 0) {
			result.FFTGainOffset = 0;
			result.Att = 0;
		}
		else if (level >= -170 && level < -20) {
			result.FFTGainOffset = 1;
			result.Att = 0;
		}

		return result;
	}

	uint32_t  GetIFATT() override { return  0; }
	uint32_t  GetRFATT() override { return  0; }
	void SetIFATT(uint32_t ifatt) override {}
};

// CM18 ģ�����?
class CM18Module : public IRFModule {
	CM18_500M_HL* impl;
public:
	CM18Module() { impl = new CM18_500M_HL(); }
	~CM18Module() override { delete impl; }
	bool SetCenterFreq(uint64_t freq) override { impl->SetCenterFreq(freq); return true; }
	RefLevelResult SetRefLevel(int level) override { 
		RefLevelResult result = { 0, 0};
		result.Att = impl->SetRefLevel(level);

		// ���������߼�
		if (level >= -50) {
			result.FFTGainOffset = 0; 
		}
		else {
			if (level >= -60) result.FFTGainOffset = 1;
			else if (level >= -70) result.FFTGainOffset = 2;
			else if (level >= -80) result.FFTGainOffset = 3;
			else if (level >= -90) result.FFTGainOffset = 4; 
		}

		return result;
	}
	void SetOutBW(int bw) override { impl->SetOutBW(bw); }
	uint32_t  GetIFATT() override { return  0; }
	uint32_t  GetRFATT() override { return  0; }
	void SetIFATT(uint32_t ifatt) override {}
};

// RPU44 Module
class RPU44Module : public IRFModule {
	RPU_44_HL* impl;
public:
	RPU44Module() { impl = new RPU_44_HL(); }
	~RPU44Module() override { delete impl; }
	bool SetCenterFreq(uint64_t freq) override { impl->SetCenterFreq(freq); return true; }
	RefLevelResult SetRefLevel(int level) override { 
		RefLevelResult result = { 0, 0};
		result.Att = impl->SetRefLevel(level); 
		return result;
	}
	void SetPowerOnOff(uint32_t flag) override { impl->PowerOnOff(flag); }
	uint32_t  GetIFATT() override { return  0; }
	uint32_t  GetRFATT() override { return  0; }
	void SetIFATT(uint32_t ifatt) override {}
};

// MZ116 ģ�����?
class MZ116Module : public IRFModule {
	HLJS386_MZ116_HL* impl;
public:
	MZ116Module() { impl = new HLJS386_MZ116_HL(); }
	~MZ116Module() override { delete impl; }
	bool SetCenterFreq(uint64_t freq) override { return impl->SetCenterFreq(freq); }
	RefLevelResult SetRefLevel(int level) override {
		RefLevelResult result = { 0, 0 };
		uint32_t attValue;
		attValue = impl->SetRefLevel(level);
		result.Att = attValue;

		if (level >= -50) {
			result.FFTGainOffset = 0;
		}
		else {
			// ������ͬ���������ƫ��?
			if (level >= -60) result.FFTGainOffset = 1;
			else if (level >= -70) result.FFTGainOffset = 2;
			else if (level >= -80) result.FFTGainOffset = 3;
			else if (level >= -90) result.FFTGainOffset = 4;

		} 
		return result;
	}
	uint32_t  GetIFATT() override { return  0; }
	uint32_t  GetRFATT() override { return  0; }
	void SetIFATT(uint32_t ifatt) override {}
};

// MZ121 ģ�����?
class MZ121Module : public IRFModule {
	HLJS386_MZ121A_HL* impl;
	bool freq_under20MHz = false;
public:
	MZ121Module() { impl = new HLJS386_MZ121A_HL(); }
	~MZ121Module() override { delete impl; }
	bool SetCenterFreq(uint64_t freq) override {
		if (freq < 20e6) {
			freq_under20MHz = true;
			impl->SetDirectFreq(freq); 
			return true;
		}
		else {
			freq_under20MHz = false;
			return impl->SetCenterFreq(freq);
		}
	}
	RefLevelResult SetRefLevel(int level) override {
		RefLevelResult result = { 0, 0};
		uint32_t attValue;

		if (freq_under20MHz) {
			attValue = impl->SetDirectRefLevel(level);
			result.FFTGainOffset = 0; 
			result.Att = attValue; 
		}
		else {
			attValue = impl->SetRefLevel(level);
			result.Att = attValue;

			if (level >= -50) {
				result.FFTGainOffset = 0; 
			}
			else {
				// ������ͬ���������ƫ��?
				if (level >= -60) result.FFTGainOffset = 1;
				else if (level >= -70) result.FFTGainOffset = 2;
				else if (level >= -80) result.FFTGainOffset = 3;
				else if (level >= -90) result.FFTGainOffset = 4;  
			}
		} 
		return result;
	}
	void SetIFATT(uint32_t ifatt) { 
		impl->SetIFATT(ifatt); 
	}
	uint32_t  GetRFATT()
	{
		return impl->GetRFATT();
	}
	uint32_t  GetIFATT()
	{ 
		return impl->GetIFATT(); 
	}
};


#pragma endregion


IRFModule* activeModule = nullptr;
RFType  RFControl::RF_SELECT ; 
RFControl::RFControl() : pcie_mem(Device::Device_MEM32::getInstance()) {} 
 

RFControl::~RFControl() {
	delete activeModule; 
}

void  RFControl::SwapIQByFreq(double CF)
{
	bool swap = isInvertedFreq(CF);

	pcie_mem->SendData(0x000C0007, swap);
}

//����Ƶģ������Ƶ����
bool  RFControl::isInvertedFreq(double freq) {
	if (RFControl::RF_SELECT == RFType::RF12) {
		return true;  // RF12���ǵ�Ƶ
	}
	else if (RFControl::RF_SELECT == RFType::MZ116) {
		return freq <= 19.4e9;
	}
	else if (RFControl::RF_SELECT == RFType::MZ121) {
		return (freq >= 20e6 && freq <= 20.3e9);
	}
	else if (RFControl::RF_SELECT == RFType::MZ121B) {
		return (freq >= 20e6 && freq < 37.00001e9);
	}
	else if (RFControl::RF_SELECT == RFType::CM18) {
		return (freq < 2e9 || (freq > 3.5e9) && freq < 8e9);
	}
	// Ĭ��Ϊ��Ƶ
	return false;
}

void RFControl::SetRFCard(RFType rf_select) {
	RF_SELECT = rf_select;
	delete activeModule;

	switch (rf_select) {
	case RFType::RF12:
		activeModule = new RF12Module();
		Global::IQFFTBaseError = -76;
		break;
	case RFType::CM18:
		activeModule = new CM18Module();
		Global::IQFFTBaseError = -88;
		break;
	case RFType::RPU44:
		activeModule = new RPU44Module();
		break;
	case RFType::MZ116:
		activeModule = new MZ116Module();
		Global::IQFFTBaseError = -97;
		break;
	case RFType::MZ121:
	case RFType::MZ121B:
		activeModule = new MZ121Module();
		Global::IQFFTBaseError = -97;
		break;
	default:
		throw std::invalid_argument("Unsupported RFType");
		break;
	}
}

bool RFControl::SetCenterFreq(uint64_t freq) {
	if (!activeModule) throw std::runtime_error("RF card not set");
	const bool result = activeModule->SetCenterFreq(freq);
	SwapIQByFreq(freq);
	return result;
}

RefLevelResult RFControl::SetRefLevel(int level) {
	if (!activeModule) throw std::runtime_error("RF card not set");
	auto result = activeModule->SetRefLevel(level);
	pcie_mem->SendData(0x10010004, result.FFTGainOffset); // Ӳ������
	return result;
}


void RFControl::SetPowerOnOff(uint32_t flag) {
	if (!activeModule) throw std::runtime_error("RF card not set");
	activeModule->SetPowerOnOff(flag);
}

void RFControl::SetOutBW(int bw) {
	if (!activeModule) throw std::runtime_error("RF card not set");
	activeModule->SetOutBW(bw);
}

uint32_t RFControl::GetIFATT() {
	 return activeModule->GetIFATT();
}
uint32_t RFControl::GetRFATT() {
	return activeModule->GetRFATT();
}
