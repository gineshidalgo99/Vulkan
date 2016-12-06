#include <fstream>      // std::ofstream
#include <iostream>     // cout, cin & cerr
#include <thread>
#include <ErrorHandler.h>





// Public functions
void ErrorHandler::handle(const ErrorCode errorCode,
                          const std::string & fileName,
                          const int codeLine,
                          const std::string & functionName,
                          const std::string & furtherDescription)
{
    // Initialize error strings (only 1st time)
    initializeErrorStrings();

    // Create error message
    auto errorMessage(getErrorDescription(errorCode,
                                          fileName,
                                          codeLine,
                                          functionName,
                                          furtherDescription));

    // Logging
    updateLog(errorCode, errorMessage);

    // Perform action
    performActionAgainstError(errorCode, errorMessage);
}

void ErrorHandler::debug(const int & line,
                         const std::string & file)
{
    // Comment to disable
//    std::cout << std::to_string(line) + " " + Utilities::getFileName(file) << std::endl;
    line;
    file;
}

void ErrorHandler::showErrorIfAnyAndExitIfCritical()
{
    std::unique_lock<std::mutex> lock1(m_errorMessageForGUIMutex);
    auto mustProgramBeExit = std::get<0>(m_errorMessageForGUI) == ErrorCode::CriticalError_00000;
    if(std::get<1>(m_errorMessageForGUI).size() != 0)
    {
        auto errorCode = std::get<0>(m_errorMessageForGUI);
        auto errorMessage(std::get<1>(m_errorMessageForGUI));
        lock1.unlock();

ErrorHandler::debug(__LINE__, __FILE__);
        //showQMessageBoxMessage(errorCode, errorMessage);

        std::unique_lock<std::mutex> lock2(m_errorMessageForGUIMutex);
        //m_errorMessageForGUI = {std::make_tuple(ErrorCode::Size, "", ErrorHandler::ErrorInOtherThreadState::Shown)};
        lock2.unlock();
    }

ErrorHandler::debug(__LINE__, __FILE__);
    if(mustProgramBeExit)
        exitProgram();
}

void ErrorHandler::exitProgram()
{
ErrorHandler::debug(__LINE__, __FILE__);
//    if(p_mainWindow != nullptr)
//        p_mainWindow->closeProgram();
//    else
        throw ExceptionToExitProgram{};
ErrorHandler::debug(__LINE__, __FILE__);
}





// Private variables initialization
std::array<std::string, (unsigned int)ErrorHandler::ErrorCode::Size> ErrorHandler::m_errorMessagesString{{}};
std::tuple<ErrorHandler::ErrorCode, std::string, ErrorHandler::ErrorInOtherThreadState> ErrorHandler::m_errorMessageForGUI{ std::make_tuple(ErrorCode::Size, std::string{""}, ErrorHandler::ErrorInOtherThreadState::Shown) };
std::mutex ErrorHandler::m_errorMessageForGUIMutex{};





// Private functions
/// @brief It performs the desired response against the error
///
/// @param errorCode Error code that help us identifying the error.
/// @param fileName Name of physical file in which the error occurred.
/// @param codeLine Number of code line in which the error occurred.
/// @param functionName Name of the function where the error ocurred.
/// @param furtherDescription Message with the error furtherDescription.
std::string ErrorHandler::getErrorDescription(const ErrorCode errorCode,
                                              const std::string & fileName,
                                              const int codeLine,
                                              const std::string & functionName,
                                              const std::string & furtherDescription)
{
    std::string message{""};

    message += "Program notification - " + m_errorMessagesString.at((unsigned int)errorCode) + ":\n";
    if (furtherDescription.size() > 0)
        message += furtherDescription + "\n";
    std::string functionNameAux{functionName};
    //Utilities::removeAllOcurrencesOfSubString(":", functionNameAux);
    message += "Technical error information: " + m_errorMessagesString.at((unsigned int)errorCode) + "-" + std::to_string(codeLine) + "-" + functionNameAux + "\n";
    fileName;
    return message;
}

void ErrorHandler::initializeErrorStrings()
{
    if (m_errorMessagesString.at((unsigned int)ErrorCode::CriticalError_00000).size() > 0)
        return;

    // Critical errors - exit program
    m_errorMessagesString.at((unsigned int)ErrorCode::CriticalError_00000) = {"Critical error"};
    // Errors (but no critical) - std::cerr (and qt message box) and throw exception
    m_errorMessagesString.at((unsigned int)ErrorCode::Error_00001) = {"(NOT USED)Error"};
    // Warnings - std::cerr
    m_errorMessagesString.at((unsigned int)ErrorCode::Warning_00002) = {"Warning"};
    // Not implemented - std::cerr (and qt message box)
    m_errorMessagesString.at((unsigned int)ErrorCode::Unimplemented_00003) = {"(NOT USED)Function / code not implemented yet."};
    // Not implemented - std::cerr (and qt message box)
    m_errorMessagesString.at((unsigned int)ErrorCode::GraphicalWarnings_00004) = {"Qt GUI warning"};
    // Information - std::cout, no output file
    m_errorMessagesString.at((unsigned int)ErrorCode::Info_00005) = {"(NOT USED)General information message"};
}

/// @brief It performs the desired response against the error
///
/// @param actionAgainstError Kind of response against the error (std::cerr, exception, assert, etc.).
/// @param errorMessage Message with the error furtherDescription.
void ErrorHandler::performActionAgainstError(const ErrorCode errorCode,
                                             const std::string & errorMessage)
{
    // std::cerr
    if(errorCode == ErrorCode::Info_00005)
        std::cout << errorMessage << std::endl;
    else
        std::cerr << errorMessage << std::endl;

    // Qt box message if: Critical error, error, unimplemented
    prepareOrExecuteShowErrorIfAnyAndExitIfCritical(errorCode, errorMessage);
}

/// @brief It performs the desired response against the error
///
/// @param actionAgainstError Decides wheter exit program or continue working.
/// @param errorMessage Message with the error furtherDescription.
void ErrorHandler::prepareOrExecuteShowErrorIfAnyAndExitIfCritical(const ErrorCode errorCode,
                                                                   const std::string & errorMessage)
{
ErrorHandler::debug(__LINE__, __FILE__);
    errorCode;
    errorMessage;
    showErrorIfAnyAndExitIfCritical();
ErrorHandler::debug(__LINE__, __FILE__);
}

void ErrorHandler::updateLog(const ErrorCode errorCode,
                             const std::string & errorMessage)
{
    //if(errorCode != ErrorCode::Info_00005)
    //{
    //    std::string fileToOpen{"logging.txt"};

    //    std::ifstream in{fileToOpen, std::ios::binary | std::ios::ate};
    //    auto currentLogSizeInBytes(in.tellg());
    //    in.close();

    //    std::ofstream logging;
    //    const auto maxLogSize = 15ull * 1024ull * 1024ull;
    //    if (currentLogSizeInBytes < maxLogSize)
    //        logging.open(fileToOpen, std::ios_base::app);
    //    else
    //        logging.open(fileToOpen, std::ios_base::trunc);

    //    // Message to write
    //    logging << "\n\n\n\n\n";
    //    logging << Utilities::getTime() + "\n";
    //    logging << errorMessage;

    //    logging.close();
    //}
}

bool ErrorHandler::codeNeedsQMessage(const ErrorCode errorCode)
{
    return {errorCode == ErrorCode::CriticalError_00000
                || errorCode == ErrorCode::Error_00001
                || errorCode == ErrorCode::Unimplemented_00003
                || errorCode == ErrorCode::GraphicalWarnings_00004};
}
