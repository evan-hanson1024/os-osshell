#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <sys/wait.h>

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    
    /************************************************************************************
     *   Example code - remove in actual program                                        *
     ************************************************************************************/
    // Shows how to loop over the directories in the PATH environment variable
    int i;
    for (i = 0; i < os_path_list.size(); i++)
    {
        printf("PATH[%2d]: %s\n", i, os_path_list[i].c_str());
    }
    /************************************************************************************
     *   End example code                                                               *
     ************************************************************************************/


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

    /************************************************************************************
     *   Example code - remove in actual program                                        *
     ************************************************************************************/
    // Shows how to split a command and prepare for the execv() function
    std::string example_command = "ls -lh";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");

    // Second example command - reuse the `command_list` and `command_list_exec` variables
    example_command = "echo \"Hello world\" I am alive!";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");
    /************************************************************************************
     *   End example code                                                               *
     ************************************************************************************/

    //printf("osshell> ");

    std::string current_command_string;
    const char* current_command;
    bool exists = false;
    std::filesystem::path orig_path;
    std::string path_to_command_string;
    const char* path_to_command;


    while (true) {
        exists = false;
        std::cout << "osshell> ";
        std::getline(std::cin, current_command_string);

        if(current_command_string == std::string("exit")) {
            exit(0);
        }

        if (!current_command_string.empty()) {
            orig_path = std::filesystem::current_path();
            current_command = current_command_string.c_str();
            for (i = 0; i < os_path_list.size(); i++) {
                std::filesystem::current_path(os_path_list[i]);
                if (std::filesystem::exists(current_command)) {
                    std::filesystem::current_path(orig_path);
                    path_to_command_string = os_path_list[i] + std::string("/") + current_command_string;
                    path_to_command = path_to_command_string.c_str();
                    //std::cout << "Current file path is: " << std::filesystem::current_path();
                    exists = true;
                    char* argv[2];
                    argv[0] = (char*)current_command;
                    argv[1] = NULL;
                    int pid = fork();
                    if (pid == 0) {
                        execv(path_to_command, argv);
                    }
                    int status;
                    waitpid(pid, &status, 0);
                    break;
                } else {
                    std::filesystem::current_path(orig_path);
                    //try next path
                }
            }

            if (exists == false) {
                printf("%s: Error command not found\n", current_command);
            }
        }
    }

    /*
    try {
        int output = system("date");
        if (output == 0) {
            std::cout << std::to_string(output) << "\n";
        } else {
            throw "";
        }
    } catch(...) {
        printf("Error command not found\n");
    }
    */

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
