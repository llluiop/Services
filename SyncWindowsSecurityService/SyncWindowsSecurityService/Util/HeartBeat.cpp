#include "HeartBeat.h"
#include "../Logger/BoostLog.h"

HeartBeat::HeartBeat(int interval)
	:interval_(interval)
	,fn_(nullptr)
	,quit_(true)
{
}

HeartBeat::~HeartBeat()
{
	fn_ = nullptr;
}

bool HeartBeat::Start(function<void()> fn)
{
	fn_ = fn;
	quit_ = false;
	t_ = thread(&HeartBeat::Beat, this);

	SLOG_INFO << "heart beat start";
	return true;
}

bool HeartBeat::Stop()
{
	quit_ = true;
	t_.join();
	fn_ = nullptr;

	SLOG_INFO << "heart beat stop";
	return true;
}

void HeartBeat::Beat()
{
	while (!quit_)
	{
		if (fn_ != nullptr)
			fn_();

		std::this_thread::sleep_for(std::chrono::seconds(interval_));
	}
}
