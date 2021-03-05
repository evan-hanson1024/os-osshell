#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <sys/wait.h>
#include <fstream>
#include <vector>
using namespace std;

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    
   


    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    std::vector<std::string> command_list; // to store command user types in, split into its variour parameters
    char **command_list_exec; // command_list converted to an array of character arrays
    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    

    int i;
    std::string current_command_string;
    const char* current_command;
	
    bool exists = false;
    std::filesystem::path orig_path;
    std::string path_to_command_string;
    const char* path_to_command;
    //open the file to save history 
    std::fstream history;
    history.open("history.txt");
    string fileContent;
    vector<string> historyVector;//create vector for history
    while(getline(history, fileContent)){
        historyVector.push_back(fileContent);
    }
    history.close();
    
    while (true) {//start of the constant loop
        exists = false;
        std::cout << "osshell> ";
        std::getline(std::cin, current_command_string);
        splitString(current_command_string, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
        
        if(command_list_exec[0] != NULL){//checks for history that the size is under 128
            historyVector.push_back(current_command_string);//holds history
            if(historyVector.size() > 130){
                historyVector.erase(historyVector.begin());
            }
        }
        if(current_command_string == std::string("exit")) {//checks exit command 
            
            std::ofstream historyLast("history.txt");
            for(int i = 0; i < historyVector.size(); i++){\
                    historyLast << (historyVector.at(i)+"\n") ;
            }
            historyLast.close();
            std::cout << "\n";
            exit(0);
        }
        std::string historyStr = "history";
        //std::cout << "the command list string 0: "<< command_list_exec[0] << std::endl;
        std::string clearStr = "clear";
        
        

        
        if(command_list_exec[0] != NULL && command_list_exec[0] == historyStr){//history command start

            //std::cout << "made it to here 1 " << std::endl;

            if(command_list_exec[1] != NULL && command_list_exec[1] == clearStr){//this checks if the extra command is clear 
                //clear command
                historyVector.clear();
                continue;

            }else if(command_list_exec[1] != NULL && isdigit(command_list_exec[1][0])){//this is for digits after
                //std::cout << "made it to here 3 " << std::endl;
                int i;
                bool clear = true;

                for(i = 1; i < strlen(command_list_exec[1]); i++){//goes through every trailing digit to make sure its a real number 
                    if(isdigit(command_list_exec[1][i])){
                        //do nothing 
                        
                    }else{
                        printf("%s: Error command not found. History command cannot have a character.\n", command_list_exec[0]);
                        clear = false;
                        break;
                    }
                }
                if(clear){//if they are all numbers then it will print starting at that number
                    
                    
                    int number = std::stoi(command_list_exec[1]);
                    if(number <=0){
                        std::cout << "Error: History expects an integer > 0 (or 'clear')" << std::endl;
                        continue;
                    }
                    historyVector.pop_back();//takes off history call so it doesnt show when called
                    if(number > historyVector.size()){//if the number is larger than 128 we dont print anything
                        continue;
                    }else{//if the number is less than 128 and more than 0 we print starting at the desired location 
                        int numberStart = historyVector.size()-number;
                        for(int i = numberStart; i < historyVector.size(); i++){
                            std::cout <<"  " << i << ":" << historyVector.at(i)<<std::endl;
                        }
                    }
                    historyVector.push_back(current_command_string);//add history command back to the log
                }

                 continue;   
            }else if(command_list_exec[1] != NULL && !isdigit(command_list_exec[1][0])){//if the command is history but the leading digit is not a number
                std::cout << "Error: History expects an integer > 0 (or 'clear')" << std::endl;
                continue;
            }
            else{//if history is alone 
                historyVector.pop_back();//takes off history 
                for(int i = 0; i < historyVector.size(); i++){//prints everything in the vector 
                    std::cout <<"  " << i << ":" << historyVector.at(i)<<std::endl;
                }
                //adds history back and freaks stuff up
                historyVector.push_back(current_command_string);
                continue;//starts the next iteration of while loop
            }
        
        }
        if (!current_command_string.empty()) {
            orig_path = std::filesystem::current_path();          
            if (command_list_exec[0][0] == '.' || command_list_exec[0][0] == '/') {
                //Do not check PATH variable
                std::filesystem::current_path(orig_path);
                if (std::filesystem::exists(command_list_exec[0])) {
                    int pid = fork();
                    if (pid == 0) {
                        execv(command_list_exec[0], command_list_exec);
                    }
                    int status;
                    waitpid(pid, &status, 0);
                    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
                } else {
                    printf("%s: Error command not found\n", command_list_exec[0]);
                    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
                }
            }else {
                for (i = 0; i < os_path_list.size(); i++) {
                    std::filesystem::current_path(os_path_list[i]);
                    if (std::filesystem::exists(command_list_exec[0])) {
                        std::filesystem::current_path(orig_path);
                        path_to_command_string = os_path_list[i] + std::string("/") + command_list_exec[0];

                        path_to_command = path_to_command_string.c_str();
                        //std::cout << "Current file path is: " << std::filesystem::current_path();
                        exists = true;
                        int pid = fork();
                        if (pid == 0) {
                            execv(path_to_command, command_list_exec);
                        }
                        int status;
                        waitpid(pid, &status, 0);
                        freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
                        break;
                    } else {
                        std::filesystem::current_path(orig_path);
                        //try next path
                    }
                }

                if (exists == false) {
                    printf("%s: Error command not found\n", command_list_exec[0]);
                    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
                }
            }
        }
    }

    return 0;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}