#include <deque>
#include <string>
#include <sstream>
#include <chrono>

namespace Core
{
    enum class Severity
    {
        Verbose,
        Info,
        Warning,
        Error
    };

    enum class Domain
    {
        APPLICATION,
        APU,
        CPU,
        MMU,
        PPU
    };

    struct Message
    {
        Severity severity;
        Domain domain;
        std::string message;
    };

    class Logger
    {
    public:
        static Logger& Instance()
        {
            static Logger s_Instance;
            return s_Instance;
        }

        void Verbose(Domain domain, std::string message)
        {
            LogMessage(Severity::Verbose, domain, message);
        }

        void Info(Domain domain, std::string message)
        {
            LogMessage(Severity::Info, domain, message);
        }

        void Warning(Domain domain, std::string message)
        {
            LogMessage(Severity::Warning, domain, message);
        }

        void Error(Domain domain, std::string message)
        {
            LogMessage(Severity::Error, domain, message);
        }

        void LogMessage(Severity severity, Domain domain, std::string message)
        {
            std::ostringstream stream;

            // append timestamp
            auto currentTime = std::chrono::system_clock::now();
            std::time_t timestamp = std::chrono::system_clock::to_time_t(currentTime);
            std::tm* timeInfo = std::localtime(&timestamp);
            stream << std::put_time(timeInfo, "[%Y-%m-%d %H:%M:%S] ");

            // append severity
            switch (severity)
            {
            case Core::Severity::Verbose:
                stream << "[VERBOSE] ";
                break;
            case Core::Severity::Info:
                stream << "[INFO] ";
                break;
            case Core::Severity::Warning:
                stream << "[WARNING] ";
                break;
            case Core::Severity::Error:
                stream << "[ERROR] ";
                break;
            }

            // append domain
            switch (domain)
            {
            case Core::Domain::APPLICATION:
                stream << "[APPLICATION] ";
                break;
            case Core::Domain::APU:
                stream << "[APU] ";
                break;
            case Core::Domain::CPU:
                stream << "[CPU] ";
                break;
            case Core::Domain::MMU:
                stream << "[MMU] ";
                break;
            case Core::Domain::PPU:
                stream << "[PPU] ";
                break;
            }

            stream << message;

            if (severity == Severity::Verbose)
            {
                m_LogMessages.push_back({ severity, domain, stream.str() });
            }

            if (m_LogMessages.size() >= MAX_MESSAGES)
            {
                m_LogMessages.pop_front();
            }
        }

        std::deque<Core::Message> GetMessages()
        {
            return m_LogMessages;
        }

    private:
        const int MAX_MESSAGES = 50000;
        std::deque<Core::Message> m_LogMessages;
    };
}