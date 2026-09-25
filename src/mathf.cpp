#include<iostream>
#include<cmath>
#include<cctype>
using namespace std;

double information_entropy(int opp[], int total){
    if(total <= 0){
        return 0.0; 
    }

    double entropy = 0.0;
    for(int i = 0; i < 256; i++){
        if(opp[i] > 0){
            double p = opp[i] / static_cast<double>(total); 
            entropy -= p * log2(p);
        }
    }
    return entropy;
}


double csqu(double o[],double e[],int k){
    if(k <= 0)
        return -1;
    double sum = 0.0;
    for(int i=0;i<k;i++){
        sum += ((o[i] - e[i]) * (o[i] - e[i])) / e[i];
    }

    return sum;
}

