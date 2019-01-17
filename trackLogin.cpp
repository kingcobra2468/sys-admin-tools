#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>
#include <regex>
#include <iostream>
#include <utility>
#include <map>

class readFile{
    public:
        virtual void parseFile(void) = 0;

        long fileSize(FILE * file){
            fseek(file, 0, SEEK_END);
            long endOfFile = ftell(file);
            rewind(file);
            return endOfFile;
        }
};

class readLogFile : public readFile{
    private: 
        FILE * file;
        std::map<std::string, int> logBuffer;
    public:
        explicit readLogFile(std::string && inputfile){
            file=fopen(inputfile.c_str(), "r");
            parseFile();
        };
        
        void parseFile(){
            char ip[20], buffer[1024]; 
            std::cmatch matched;
            std::regex match("[1-9]{1,}[.][1-9]{1,}[.][1-9]{1,}[.][1-9]{1,}");
            if (file != NULL){
            int i = 0;
            long end = fileSize(file);
            while(ftell(file) != end){    
                fgets(buffer, sizeof(buffer), file);
                std::regex_replace(buffer, static_cast<std::regex>("\n"), "");
                std::regex_search( buffer, matched, match);
                if (matched[0].str().size() != 0) {
                    if (logBuffer.count( matched[0].str()) == 0){
                        logBuffer.insert(std::pair<std::string, int>(matched[0].str(), 1));
                    }
                    else{
                        logBuffer.at(matched[0].str())+=1;
                        }
                    }   
                }
            }
        }
       std::map<std::string, int> getLogBuf(){
           return logBuffer;
       }
       ~readLogFile(){
            fclose(file);
        } 
};
class readSavedHostFile : public readFile {
    private: 
        FILE * file;
        std::map<std::string, int> savedHosts;
        std::string fileName;
    public:
        explicit readSavedHostFile(std::string && inputfile) : fileName(inputfile){
            if (!std::ifstream(inputfile).good()){
                std::ofstream(inputfile).close();
            }
            file=fopen(inputfile.c_str(), "r+");
            parseFile();
        };
        void parseFile(){
            int occurances = 0;
            char userAgentBuffer[1024]; 
            long end = fileSize(file);
            while(ftell(file) != end){    
                fscanf(file, "%s %i", userAgentBuffer, &occurances);
                savedHosts.insert(std::pair<std::string, int>(userAgentBuffer, occurances));
            }
            fclose(file);
            file = nullptr;
        }
        bool isFound(std::pair<std::string, int> && tmp){
            return (savedHosts.count(tmp.first) > 0);
        }
        void addHost(std::pair<std::string, int> && tmp){
           savedHosts.emplace(tmp.first, tmp.second);
        }
        void newOccurance(std::pair<std::string, int> && tmp){
            savedHosts.at(tmp.first) +=tmp.second;
        }
        ~readSavedHostFile(){
            file=fopen(fileName.c_str(), "w+");
            for (auto it = savedHosts.begin(); it != savedHosts.end(); it++){
                fprintf(file, "%s %i\n", it->first.c_str(),it->second);
                fflush(file); 
            }
            fclose(file);
        }
        /*std::list<std::pair<std::string, std::string>> * getSavedHost(){
            return &savedHosts;
        } 
        */  
};

class checker{
    private:
        readLogFile * log;
        readSavedHostFile * hosts;
    public:
        explicit checker(readLogFile * ptrLog, readSavedHostFile * readPtrLog) : log(ptrLog), hosts(readPtrLog) {};
        void updateHost(){
            for(auto x : log->getLogBuf()){
                if (!hosts->isFound(x)) {
                    hosts->addHost(x);
                }
                else{
                    hosts->newOccurance(x);
                }
            }
        }
};

int main(){
    readLogFile * logs = new readLogFile("/home/erik/Documents/Programming/CPP/auth.log"); //auth.log.1
    readSavedHostFile * hosts = new readSavedHostFile("/home/erik/Documents/Programming/CPP/knownIP.txt");
    checker t(logs, hosts);
    t.updateHost();
    delete hosts;
}

/*

for(auto x= logBuffer.begin(); x != logBuffer.end(); ++x){
                std::cout << x->first << std::endl;
            }
            std::cout << logBuffer.size() << std::endl;


*/
