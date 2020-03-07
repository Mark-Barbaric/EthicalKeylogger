#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <thread>
#include <memory>
#include <chrono>

class TimerBase
{

};

class InfiniteTimer
{

};

class Timer
{
    std::unique_ptr<std::thread> thread;
    bool alive = false; //will check if timer is running
    long callNumber = -1L; //counts how many times we would like to run a function (ie: send mail funciton should be called 5x)
    long repeat_count = -1L; //counts how many times a certain function has been called (ie: send mail function has been called 2x so far)
    std::chrono::milliseconds interval = std::chrono::milliseconds(0);
    std::function<void(void)> funct = nullptr;

    void sleepAndRun()
    {
        std::this_thread::sleep_for(interval);
        if(alive)
        {
            Function()();  //double parentheses: 1st pair calls the function to return a function
                            //second pair calls the function the 1st function returns
        }
    }

    void threadFunc()
    {
        if(callNumber == Infinite)
        {
            while(alive)
            {
                sleepAndRun();
            }
        }
        else
        {
            while(repeat_count--)
            {
                sleepAndRun();
            }

        }
    }

public:

	static const long Infinite = -1L;

	Timer() = default;
    Timer(const std::function<void(void)> &f,
          const unsigned long &i,
          const long repeat = Timer::Infinite) 
		: funct(f),
         interval(std::chrono::milliseconds(i)),
		callNumber(repeat)
	{}

    void start()
    {
        if(isAlive())
        {
            return;
        }
        alive = true;
        repeat_count = callNumber;
        thread.reset(new std::thread(&Timer::threadFunc, this));
    }

    void stop()
    {
        alive = false;
        thread.get()->join();
    }

    void setFunction(const std::function<void(void)> &f)
    {
        funct = f;
    }

	void setInterval(const unsigned long &i)
	{
		if (alive)
		{
			return;
		}

		interval = std::chrono::milliseconds(i);
	}

	void repeatCount(const long r)
	{
		if (alive)
		{
			return;
		}

		callNumber = r;
	}

    //is the timer running or not?
    bool isAlive() const {return alive;}
    long getLeftCount() const {return repeat_count;}
	long getRepeatCount() const { return callNumber; }
    unsigned long Interval() const {return interval.count();}
	const std::function<void(void)> &Function() const { return funct; }
};

#endif // TIMER_H_INCLUDED
