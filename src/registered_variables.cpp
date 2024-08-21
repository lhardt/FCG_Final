#include "registered_variables.h"

#include <map>
#include <string>

#include "logger.h"



TypingMode typingModeOf(RegisteredVariableType rvt){
    if(rvt == R_BOOL) return EDIT_BOOL;
    if(rvt == R_STRING) return EDIT_STRING;
    if(rvt == R_FLOAT) return EDIT_FLOAT;
    
    log_severe("Could not find TypingMode for RVT %d.", (int)rvt);
    std::exit(-1);
}

std::string typingModeToStr(TypingMode t){
    if(t == tm_PLAY) return "PLAY";
    if(t == tm_WRITE) return "WRITE";
    if(t == EDIT_BOOL) return "bool";
    if(t == EDIT_STRING) return "string";
    if(t == EDIT_FLOAT) return "float";
    
    return "???";
}

RegisteredVariable makeBoolRef(bool* v){
    return (RegisteredVariable){
        .type = R_BOOL, 
        .br_value = v
    };
}

RegisteredVariable makeFloatRef(float* v){
    return (RegisteredVariable){
        .type = R_FLOAT, 
        .fr_value = v
    };
}

RegisteredVariable makeStringRef(std::string* v){
    return (RegisteredVariable){
        .type = R_STRING, 
        .sr_value = v
    };
}

std::string variableTypeToStr(RegisteredVariableType t){
    if(t == R_BOOL) return "bool";
    if(t == R_FLOAT) return "float";
    if(t == R_STRING) return "string";
    return "???";
}


bool string_to_float(std::string s, float * val){
    if(s.size() == 0) return false;
    try {
        *val = std::stof(s);
        return true;
    } catch (const std::exception& ex) {
        log_info("Could not convert [%s] to float.", s.c_str());
        return false;
    }
}

void handleValueInput(std::string input, RegisteredVariable& variable){
    bool null_ref = false; bool invalid_input = false;
    switch(variable.type){
        case R_BOOL:{        
            bool * ref = variable.br_value;
            if(ref == NULL){ null_ref = true; break;}
            if(input.size() == 0 || (input[0] != 'y' && input[0] != 'Y' && input[0] != 'n'  && input[0] != 'N') ){
                invalid_input = true;
                break;
            }
            bool value = (input[0] == 'y'  || input[0] == 'Y');
            *ref = value; input = std::to_string(value);
            break;
        }
        case R_FLOAT:{
            float * ref = variable.fr_value;
            float value;
            if(!string_to_float(input, &value)){ invalid_input = true; break; }
            *ref = value; input = std::to_string(value);
            break;
        }
        case R_STRING:{
            std::string * ref = variable.sr_value;
            if(ref == NULL){ null_ref = true; break;}
            *ref = input; 
            break;
        }
    }
    if(null_ref) {
        log_error("Tried to update variable [%s] with a null [%s] reference! Ignoring.");
        return;
    }
    if(invalid_input){
        log_info("Invalid input for %s variable : [%d]. Ignoring", variableTypeToStr(variable.type).c_str(), input);    
        return;
    }

    log_info("Success! %s variable is set to [%s].", variableTypeToStr(variable.type).c_str(),  input.c_str());    

    // Continue in the same mode?
    // g_typingMode = PLAY;
}


std::string variableValueToString(RegisteredVariable r){
    switch(r.type) {
        case  R_BOOL:
            log_assert(NULL != r.br_value, "");
            return (* r.br_value) ? "Yes" : "No";
        case  R_FLOAT:
            log_assert(NULL != r.fr_value, "");
            return std::to_string(* r.fr_value);
        case  R_STRING:
            log_assert(NULL != r.sr_value, "");
            return * r.sr_value;
    }
    return "???";
} 
