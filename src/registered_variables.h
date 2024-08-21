#ifndef REGISTERED_VARIABLES_H
#define REGISTERED_VARIABLES_H

#include <string>
#include <map>

enum RegisteredVariableType { 
    R_BOOL,
    R_FLOAT,
    R_STRING,
};

struct RegisteredVariable {
    RegisteredVariableType type;
    
    float * fr_value;
    bool * br_value;
    std::string * sr_value;
    
    float fvalue;
    bool  bvalue;
    std::string svalue;
};

std::string variableTypeToStr(RegisteredVariableType t);
RegisteredVariable makeBoolRef(bool* v);
RegisteredVariable makeFloatRef(float* v);
RegisteredVariable makeStringRef(std::string* v);

std::string variableValueToString(RegisteredVariable r);

enum TypingMode {
    tm_PLAY = 1, /** Normal game controls */
    tm_WRITE = 2, /** Writing a command */
    /** Writing a varible or editing */
    EDIT_BOOL = 3,  
    EDIT_STRING = 4,
    EDIT_FLOAT = 5
};

TypingMode typingModeOf(RegisteredVariableType rvt);

std::string typingModeToStr(TypingMode t);

bool string_to_float(std::string s, float * val);

void handleStringCommand(std::string command, std::map<std::string, RegisteredVariable> &registeredVariables);
void handleValueInput(std::string input, RegisteredVariable& variable);



#endif /** REGISTERED_VARIABLES_H */
