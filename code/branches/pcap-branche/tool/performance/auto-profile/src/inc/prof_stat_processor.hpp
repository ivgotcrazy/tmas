#ifndef BROADINTER_PROF_STAT_PROCESSOR
#define BROADINTER_PROF_STAT_PROCESSOR

#include "pkt_processor.hpp"

namespace BroadInter
{

template<class Next, class Succ>
class L3ProfStatProcessor : public PktProcessor<L3ProfStatProcessor<Next, Succ>, Next, Succ>
{
public:
	ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);
};

template<class Next, class Succ>
class L4ProfStatProcessor : public PktProcessor<L4ProfStatProcessor<Next, Succ>, Next, Succ>
{
public:
	ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);
};

}

#include "prof_stat_processor-inl.hpp"

#endif