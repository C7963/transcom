#pragma once

#include "PscanDefs.h"
#include "Sweep.h"

namespace SWEEPCONFIG
{
    // Single configuration transaction for the PSCAN path.
    class SpectrumSweep
    {
    public:
        explicit SpectrumSweep(Sweep* sweep);

        bool Config(const PSCANCONFIG::PscanConfig& config, const char* reason);

    private:
        Sweep* sweep_;
    };
}
