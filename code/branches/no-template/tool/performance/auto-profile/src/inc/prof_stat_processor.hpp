#ifndef BROADINTER_PROF_STAT_PROCESSOR
#define BROADINTER_PROF_STAT_PROCESSOR

#include "pkt_processor.hpp"

namespace BroadInter
{

class L3ProfStatProcessor : public PktProcessor
{
private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;
};

class L4ProfStatProcessor : public PktProcessor
{
private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;
};

class L7ProfStatProcessor : public PktProcessor
{
private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;
};

}


#endif