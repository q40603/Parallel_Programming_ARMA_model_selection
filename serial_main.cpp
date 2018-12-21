//
// Created by yue on 18-3-16.
//
#include <vector>
#include "cstdio"
#include "iostream"
#include "ARIMAModel.h"
#include <chrono>

#define DATA_LIMIT 170

int main(int argc, char** argv){
    if(argc < 2){
        printf("Usage: %s <dataset>\n", argv[0]);
        return 0;
    }
    freopen(argv[1],"r",stdin);

    auto total_start = std::chrono::system_clock::now();

    int data_count = 1;
    std::vector<double> dataArray;
    double input;
   	while(std::cin>>input){
        dataArray.push_back(input);
        if(++data_count > DATA_LIMIT) break;
    }
    printf("Data Size: %u\n", dataArray.size());
    double expect = dataArray.back();
    dataArray.pop_back();
    
    auto cpt_start = std::chrono::system_clock::now();

    /*****  Computation  *****/
    ARIMAModel arima(dataArray);
    std::vector<int> bestModel = arima.getARIMAModel(0, 1);
    int bestPredict = arima.predictValue(bestModel[0], bestModel[1]);

    auto cpt_end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = cpt_end-cpt_start;
    printf("Computation time: %lf\n", elapsed_seconds.count());

    /***** Result *****/
    printf("Best model:\n");
    printf("p: %d,\tq: %d\n", bestModel[0], bestModel[1]);
    printf("Predict: %d,\tExpect: %lf\n", bestPredict, expect);
    auto total_end = std::chrono::system_clock::now();
    elapsed_seconds = total_end - total_start;
    printf("Total time: %lf\n", elapsed_seconds.count());
}