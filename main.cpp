//
// Created by yue on 18-3-16.
//

#include <vector>
#include "cstdio"
#include "iostream"
#include "ARIMAModel.h"


int main(){
    freopen("data.txt","r",stdin);
    double gets;
    std::vector<double> dataArray;
    while(std::cin>>gets){
        dataArray.push_back(gets);
        //std::cout<<gets<<std::endl;
    }
    for(int p = 1 ; p < 10 ; p++){
        for(int q = 1 ; q < 10 ; q++){
            ARMAModel* arma = new ARMAModel(dataArray, p, q);
            std::vector<std::vector<double>> coe;
            coe=arma->solveCoeOfARMA();
            ARMAMath ar_math;
            double aic = ar_math.getModelAIC(coe, dataArray, 3); 
            std::cout<<p<<" "<<q<<" aic : "<< aic<<std::endl;     
        }
    }


}