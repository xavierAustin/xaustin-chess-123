#include <iostream>
#include <fstream>
#include <ctime>

enum LOGLEVEL {
    //magic numbers based on colors of text printed to console
    //gray
    INFO = 8, 
    //red
    ERR = 12, 
    //yellow
    WARN = 14,
    //white
    DEFAULT = 15
};

class Logger {
    int level;
    std::time_t currentTime;
    public:
    void initalize(enum LOGLEVEL initLevel = LOGLEVEL :: INFO){
        //set up windows console handler
        //declare that logging has been initialized
        setLevel(LOGLEVEL :: DEFAULT);
        log("\n--New Logging Session Initialized--\n",1,1);
        //set level to initialized value
        setLevel(initLevel);
    }
    void setLevel(enum LOGLEVEL setLevel = LOGLEVEL :: INFO){
        this->level = setLevel;
    }
    void log(const auto strToWrite, bool hideDesc = 0, bool hideTime = 0){
        std::string desc = hideDesc ? "" : (this->level == LOGLEVEL :: WARN) ? "WARN: " : (this->level == LOGLEVEL :: ERR) ? "ERROR: " : "INFO: ";
        //create and open log.txt
        std::fstream logFile;
        logFile.open("log.txt",std::fstream::app);
        //write to the file
        currentTime = std::time(nullptr);
        if (logFile.is_open() && !hideTime)
            logFile << std::asctime(std::localtime(&currentTime));
        if (logFile.is_open()){
            logFile << desc << strToWrite << std::endl;
            //close the file when done
            logFile.close();
        }else{
            std::cout << "WARN: Error writing to log.txt!\n Not enough space perhaps?" << std::endl;
        }
        //write to console
        if (!hideTime)
            std::cout << std::asctime(std::localtime(&currentTime));
        std::cout << desc << strToWrite << std::endl;
        //if on windows return terminal color to default
    }
};