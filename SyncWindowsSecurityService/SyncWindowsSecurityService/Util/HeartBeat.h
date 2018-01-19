#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <functional>
#include <thread>

using namespace std;

class HeartBeat
{
public:
	HeartBeat(int interval);
	~HeartBeat();

	//just sleep interval seconds when fn was not set
	bool Start(function<void()> fn = nullptr);
	bool Stop();

private:
	void Beat();

private:
	int interval_;
	function<void()> fn_;
	thread t_;
	bool quit_;

private:
	HeartBeat(const HeartBeat&) = delete;
	HeartBeat& operator=(const HeartBeat&) = delete;
};


#endif
