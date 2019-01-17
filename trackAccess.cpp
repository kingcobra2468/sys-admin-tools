#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>
#include <regex>
#include <iostream>
#include <utility>
#include <map>
#include <vector>


class readFile{
    public:
        virtual void parseFile(void) = 0;

        long fileSize(FILE * file){
            fseek(file, 0, SEEK_END);
            long endOfFile = ftell(file);
            rewind(file);
            return endOfFile;
        }

        bool checkVector(std::vector<std::string> inputVector, std::string inputAgent){
            bool found = false;
            for(std::vector<std::string>::iterator iter = inputVector.begin(); iter != inputVector.end(); iter++){
                if (static_cast<std::string>(*iter).compare(inputAgent) == 0){
                    return true;
                    break;
                }
            }
            return false;
        }
};

class readLogFile : public readFile{
    private: 
        FILE * file;
        std::map<std::string, std::vector<std::string>> logBuffer;
    public:
        explicit readLogFile(std::string && inputfile){
            file=fopen(inputfile.c_str(), "r");
            parseFile();
        };
        void parseFile(){
            char ip[20], buffer[1024]; 
            std::cmatch matched;
            std::regex match("\".\\w+/.*\\s.*$");
            if (file != NULL){
            int i = 0;
            long end = fileSize(file);
            while(ftell(file) != end){    
                fscanf(file, "%s", ip);
                fgets(buffer, sizeof(buffer), file);
                std::regex_replace(buffer, static_cast<std::regex>("(\n|\r)"), "");
                std::regex_search( buffer, matched, match);
                if (strlen(ip) != 0 && matched[0].str().size() > 20) {
                    std::vector<std::string> temp;
                    if(logBuffer.count(ip) == 0){
                       // std::cout << "here\n";
                        temp.push_back(matched[0].str());
                        logBuffer.insert(std::pair<std::string, std::vector<std::string>>(ip, std::move(temp)));
                    }
                    else if (logBuffer.count(ip) == 1){
                        //std::cout << "poo\n";
                        if (!checkVector(logBuffer.at(ip), matched[0].str())){
                            temp = logBuffer.at(ip);
                            temp.push_back(matched[0].str());
                            logBuffer.at(ip) = move(temp);
                            }
                        }
                    }
                }
            }
        }
       std::map<std::string, std::vector<std::string>> getLogBuf(){
           return logBuffer;
       }
       ~readLogFile(){
            fclose(file);
        } 
};
class readSavedHostFile : public readFile {
    private: 
        FILE * file;
        std::map<std::string, std::vector<std::string>> savedHosts;
    public:
        explicit readSavedHostFile(std::string && inputfile){
            if (!std::ifstream(inputfile).good()){
                std::ofstream(inputfile).close();
            }
            file=fopen(inputfile.c_str(), "r+");
            parseFile();
        };
        void parseFile(){
            char ipBuffer[20], userAgentBuffer[1024]; 
            long end = fileSize(file);
            std::vector<std::string> temp;
            while(ftell(file) != end){    
                fscanf(file, "%s", ipBuffer);
                fgets(userAgentBuffer, sizeof(userAgentBuffer), file);
                std::regex_replace(userAgentBuffer, static_cast<std::regex>("(\n|\r)"),"\0");
                if(savedHosts.count(ipBuffer) == 0){
                    temp.push_back(static_cast<std::string>(userAgentBuffer).erase(0,1));
                    savedHosts.insert(std::pair<std::string, std::vector<std::string>>(ipBuffer, std::move(temp)));
                    }
                else if (savedHosts.count(ipBuffer) == 1){
                    if (!checkVector(savedHosts.at(ipBuffer), static_cast<std::string>(userAgentBuffer).erase(0,1))){
                        std::cout << ipBuffer << " " << userAgentBuffer << std::endl;
                        temp = savedHosts.at(ipBuffer);
                        temp.push_back(static_cast<std::string>(userAgentBuffer).erase(0,1));
                        std::cout << temp.size() << std::endl;
                        savedHosts.at(ipBuffer) = std::move(temp);
                    }
                }
                //savedHosts.insert(std::pair<std::string,std::string>(static_cast<std::string>(userAgentBuffer).erase(0,1), ipBuffer));
            }
        }
        bool isFound(std::string ip, std::string userAgent){
            if (savedHosts.count(ip) == 0){
                return false;
            }
            else{
            return checkVector(savedHosts.at(ip), userAgent) ? true : false;
            }
        }
        void addHost(std::string ip, std::string userAgent){
           fprintf(file, "%s %s", ip.c_str(), userAgent.c_str());
           fflush(file); 
        }
        ~readSavedHostFile(){
            fclose(file);
        }
 
};

class checker{
    private:
        readLogFile * log;
        readSavedHostFile * hosts;
    public:
        explicit checker(readLogFile * ptrLog, readSavedHostFile * readPtrLog) : log(ptrLog), hosts(readPtrLog) {};
        void updateHost(){
            for(auto x : log->getLogBuf()){
                for (std::string userAgent : x.second){
                    std::cout << x.first <<  " " << userAgent[-1]  << "END " << std::endl;
                    if (!hosts->isFound(x.first, userAgent)) hosts->addHost(x.first, userAgent);
                }
            }
        }
};

int main(){
    readLogFile * logs = new readLogFile("/home/erik/Documents/Programming/CPP/access.log");
    readSavedHostFile * hosts = new readSavedHostFile("/home/erik/Documents/Programming/CPP/knownHosts.txt");
    checker t(logs, hosts);
    t.updateHost();
}

/*

for(auto x= logBuffer.begin(); x != logBuffer.end(); ++x){
                std::cout << x->first << std::endl;
            }
            std::cout << logBuffer.size() << std::endl;


*/
