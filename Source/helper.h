#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

#include <string>
#include <ctime>
#include <sstream>
#include <fstream>

//Helper.h has general purpose functions that will be used throughout the project
namespace Helper
{
	inline std::tm localtime_xp(std::time_t time);

    template <class T>
    std::string toSTDString(const T &);

    struct DateTime
    {
		int D, m, y, H, M, S;

        DateTime()
        {
			auto tm = localtime_xp(std::time(0));
            D = tm.tm_mday; //day
            m = tm.tm_mon +1; //month +1 since counting starts at 0
            y = 1900 + tm.tm_year; // year
            M = tm.tm_min; //minutes
            H = tm.tm_hour; //hours
            S = tm.tm_sec; //seconds
        }

        DateTime(int D, int m, int y, int H, int M, int S) 
			: D(D), m(m), y(y), H(H), M(M), S(S) {}
        DateTime(int D, int m, int y) 
			: D(D), m(m), y(y), H(0), M(0), S(0) {}


        DateTime Now() const{return DateTime();}

        std::string GetDateString() const{
			return std::string(D < 10 ? "0" : "") + 
				toSTDString(D) + 
				std::string(M < 10 ? ".0" : ".") + 
				toSTDString(m) + "." + toSTDString(y);}

        std::string GetTimeString(const std::string &sep = ":") const
        {
            return std::string(H < 10 ? "0" : "") + toSTDString(H) + sep +
                   std::string(M < 10 ? "0" : "") + toSTDString(M) + sep +
                   std::string(S < 10 ? sep : "") + toSTDString(S);
        }

        std::string GetDateTimeString(const std::string &sep = ":") const
        {
            return GetDateString() + " " + GetTimeString(sep);
        }
    };

	inline std::tm localtime_xp(std::time_t timer)
	{
		std::tm bt{};
#if defined(__unix__)
		localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
		localtime_s(&bt, &timer);
#else
		static std::mutex mtx;
		std::lock_guard<std::mutex>lock(mtx);
		bt = *std::Localtime(&timer);
#endif
		return bt;
	}

    template <class T>
    std::string toSTDString(const T &e)
    {
        std::ostringstream s;
        s << e;
        return s.str();
    }

    //writes to the log
    void WriteAppLog(const std::string &s)
    {
        std::ofstream file("AppLog.txt", std::ios::app);
        file << "[" << Helper::DateTime().GetDateTimeString() << "]" <<
        "\n" << s << std::endl << "\n";
        file.close();
    }

}


#endif // HELPER_H_INCLUDED
