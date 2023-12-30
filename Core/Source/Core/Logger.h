#include <vector>
#include <string>

namespace Core
{
    enum class LogMessageType
    {
        Info,
        Error,
        CPU,
        MMU,
        PPU
    };

    struct LogMessage
    {
        LogMessageType type;
        std::string message;
    };

    class Logger
    {
    public:
        static Logger& Instance()
        {
            static Logger instance;
            return instance;
        }

        void LogCPUMessage(std::string message)
        {
            LogMessage(LogMessageType::CPU, message);
        }

        void LogMessage(LogMessageType type, std::string message)
        {
            logMessages.push_back({ type, message });

            if (logMessages.size() >= MAX_MESSAGES)
            {
                logMessages.erase(logMessages.begin());
            }
        }

        std::vector<Core::LogMessage> GetMessages()
        {
            return logMessages;
        }

    private:
        const int MAX_MESSAGES = 500;
        std::vector<Core::LogMessage> logMessages;
    };
}