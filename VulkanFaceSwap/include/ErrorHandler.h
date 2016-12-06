#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <array>
#include <mutex>
#include <tuple>
#include <string>

class ErrorHandler
{
    public:
        class ExceptionToExitProgram{};

        /// @brief enum classes that specifies the error occurred. It also specifies the kind of response against the error (std::cerr, exception, assert, etc.).
        enum class ErrorCode : unsigned int
        {
        // Critical errors - exit program
            CriticalError_00000 = 0,
        // Errors (but no critical) - and qt message box
            Error_00001 = 1,
        // Warnings - only std::cerr
            Warning_00002 = 2,
        // Unimplemented - qt message box
            Unimplemented_00003 = 3,
        // Warnings with QtMessageBox - qt message box
            GraphicalWarnings_00004 = 4,
        // Information - std::cout, no output file
            Info_00005 = 5,
        // Size
            Size,
        };
        enum class ErrorInOtherThreadState : unsigned int
        {
            NotShownYet,
            Showing,
            Shown,
        };

        /// @brief It manages all the errors of the program. It also provokes the program to show the error.
        ///
        /// @param errorCode Error code that help us identifying the error.
        /// @param fileName Name of physical file in which the error occurred.
        /// @param codeLine Number of code line in which the error occurred.
        /// @param functionName Name of the function where the error ocurred.
        /// @param furtherDescription Message with the error furtherDescription.
        static void handle(const ErrorCode errorCode,
                           const std::string & fileName,
                           const int codeLine,
                           const std::string & functionName,
                           const std::string & furtherDescription);
        /// @brief It creates the defaulf error strings of the program. This function will be automatically called at the first use of this static class. However, it should be initialize at the beginning of the program.
        static void debug(const int & line,
                          const std::string & file);
        static void showErrorIfAnyAndExitIfCritical();
        static void exitProgram();



    private:
        // Private variables
        static std::array<std::string, (unsigned int)ErrorHandler::ErrorCode::Size> m_errorMessagesString;
        static std::tuple<ErrorCode, std::string, ErrorInOtherThreadState> m_errorMessageForGUI;
        static std::mutex m_errorMessageForGUIMutex;
        // Private functions
        static std::string getErrorDescription(const ErrorCode errorCode,
                                               const std::string & fileName,
                                               const int codeLine,
                                               const std::string & functionName,
                                               const std::string & furtherDescription = "");
        static void initializeErrorStrings();
        static void performActionAgainstError(const ErrorCode errorCode,
                                              const std::string & errorMessage);
        static void prepareOrExecuteShowErrorIfAnyAndExitIfCritical(const ErrorCode errorCode,
                                                                    const std::string & errorMessage);
        static void updateLog(const ErrorCode errorCode,
                              const std::string & errorMessage);
        static bool codeNeedsQMessage(const ErrorCode errorCode);
};

#endif // ERROR_HANDLER_H
