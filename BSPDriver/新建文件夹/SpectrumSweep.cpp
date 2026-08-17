#include "SpectrumSweep.h"

#include <fstream>
#include <sstream>
#include <windows.h>

namespace
{
    std::string GetSpectrumSweepDllDir()
    {
        char path[MAX_PATH] = { 0 };
        HMODULE module = nullptr;
        if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&GetSpectrumSweepDllDir), &module))
        {
            GetModuleFileNameA(module, path, MAX_PATH);
        }

        std::string fullPath(path);
        const auto pos = fullPath.find_last_of("\\/");
        return pos == std::string::npos ? "." : fullPath.substr(0, pos);
    }

    void SpectrumSweepLog(const char* message)
    {
        std::ofstream log(GetSpectrumSweepDllDir() + "\\bsdriver_diag.log", std::ios::app);
        if (log.is_open())
        {
            log << message << '\n';
        }
    }
}

namespace SWEEPCONFIG
{
    SpectrumSweep::SpectrumSweep(Sweep* sweep)
        : sweep_(sweep)
    {
    }

    bool SpectrumSweep::Config(const PSCANCONFIG::PscanConfig& config, const char* reason)
    {
        if (!sweep_)
        {
            SpectrumSweepLog("[SPECTRUM_SWEEP_CONFIG] missing Sweep instance");
            return false;
        }

        std::ostringstream begin;
        begin << "[SPECTRUM_SWEEP_CONFIG] begin"
              << " reason=" << (reason ? reason : "unknown")
              << " CF=" << config.centerFreq
              << " Span=" << config.span
              << " RBW=" << config.rbw
              << " Step=" << config.step
              << " RefLevel=" << config.refLevel;
        SpectrumSweepLog(begin.str().c_str());

        // Match the C# PSCAN flow: assign all sweep fields first, then commit
        // Config once. C# applies RF/IF attenuation after this step through its
        // PSCAN front-end command sequence; RefLevel is not that hardware gain.
        sweep_->SetCFSpan(config.centerFreq, config.span);
        sweep_->SetRBW(config.rbw);
        sweep_->SetStep(config.step);
        Global::RefLevel = config.refLevel;
        SpectrumSweepLog("[SPECTRUM_SWEEP_CONFIG] RefLevel retained as PSCAN display state; RF/IF write deferred to Pscan front-end");
        sweep_->Config();

        std::ostringstream end;
        end << "[SPECTRUM_SWEEP_CONFIG] end"
            << " reason=" << (reason ? reason : "unknown")
            << " points=" << sweep_->GetDataLen();
        SpectrumSweepLog(end.str().c_str());
        return true;
    }
}
