#include "renamer.h"
#include "mathf.h"
#include<iostream>

std::string r_get_name(std::string file_name){
}

bool r_is_eng(std::string str){
    return true;
}


double r_alpha_bigram(std::string str){
    double o[26] = {0};
    int total = 0;
    for(char c : str){
        if(isalpha(c)){
            int idx = static_cast<unsigned char>(std::tolower(c));
            o[idx - 'a']++;
            total++;
        }
    }
    double e[26] = {0};
    for(int i=0;i<26;i++)
        e[i] = ENG_FREQ[i] * static_cast<double>(total);
    return csqu(o,e,26) / total;
}
