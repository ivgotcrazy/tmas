#ifndef BROADINTER_PKT_CAPTURE_SIMULATOR
#define BROADINTER_PKT_CAPTURE_SIMULATOR

#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

class PressureTestSimulator
{
public:
    PressureTestSimulator(const PktHandler& handler);

    void Start();
	void Stop();

private:
	void GenPktThreadFunc();

private:
	bool stop_flag_;

	PktHandler pkt_handler_;
};

class ProfMeasureSimulator
{
public:
	ProfMeasureSimulator(const PktProcessorSP& processor);

	void Start();

private:
	PktProcessorSP pkt_processor_;
};


}

#endif
