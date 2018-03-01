#ifndef BROADINTER_PKT_CAPTURE_SIMULATOR
#define BROADINTER_PKT_CAPTURE_SIMULATOR

#include "tmas_typedef.hpp"
#include "message.hpp"
#include "eth_monitor.hpp"
#include "prof_typedef.hpp"

namespace BroadInter
{

class PressureTestSimulator
{
public:
    PressureTestSimulator(const PktDispatcherTestTypeSP& dispatcher);

    void Start();
	void Stop();

private:
	void GenPktThreadFunc();

private:
	bool stop_flag_;

	PktDispatcherTestTypeSP pkt_dispatcher_;
};

class ProfMeasureSimulator
{
public:
	ProfMeasureSimulator(const EthMonitorTestTypeSP& eth_monitor);

	void Start();

private:
	EthMonitorTestTypeSP eth_monitor_;
};


}

#endif
