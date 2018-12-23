//
// Created by yue on 18-3-16.
//
#include <pthread.h>
#include <vector>
#include "cstdio"
#include "iostream"
#include "ARIMAModel.h"
#include <chrono>
#include <ctime>  
#include <semaphore.h>
/*
void get5minsData(&vector<double> dataArray, char *filename)
{

}
*/

typedef struct parameter
{
    pthread_t thread_id;
    int thread_num;
    int start;
    int aic;
    int p;
    int q;
    double expect;
    double predict;
    double time;
}parameter;

std::vector<double> dataArray;

void *thread_f(void *thread_p)
{
   // auto s = std::chrono::system_clock::now();
    auto s = std::chrono::system_clock::now();
    //std::vector<double> local_dataArray = dataArray;
    int start = ((parameter*)thread_p) -> start;
    int thread_num = ((parameter*)thread_p) -> thread_num;
    ARIMAModel *arima = new ARIMAModel(dataArray);
    arima->expect = ((parameter*)thread_p) -> expect;
    std::vector<int> bestModel;
    std::vector<std::vector<int>> a;
    bestModel = arima->getARIMAModel(0, a, false, start, thread_num);
    ((parameter*)thread_p) -> p = bestModel[0];
    ((parameter*)thread_p) -> q = bestModel[1];
    ((parameter*)thread_p) -> aic = bestModel[2];

    ((parameter*)thread_p) -> predict = arima->predictValue(bestModel[0], bestModel[1]);

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-s;
    ((parameter*)thread_p) -> time = elapsed_seconds.count();
}



int main(int argc, char** argv){
    freopen(argv[1],"r",stdin);
    double gets;
    //std::vector<double> dataArray;

    int thread_num = std::stoi(argv[2]);
    parameter thread_parameters[thread_num - 1];
    


  	/* 
   	while(std::cin>>gets){
        dataArray.push_back(gets);
    }
    */
    for(int i = 1; i <= 100; i++)
    {
    	std::cin>>gets;
    	dataArray.push_back(gets);
    }
    //std::cout<<"Hello"<<std::endl;
    double expect = dataArray.back();

    for(int i = 0; i < thread_num - 1; i++)
    {
        thread_parameters[i].thread_num = thread_num;
        thread_parameters[i].start = i;
        thread_parameters[i].expect = expect;
        pthread_create(&thread_parameters[i].thread_id, NULL, thread_f, (void *)&thread_parameters[i]); 
    }

    int start = thread_num - 1;
    //int thread_num = ((parameter*)thread_p) -> thread_num;
    ARIMAModel *arima = new ARIMAModel(dataArray);
    arima->expect = expect;
    std::vector<int> bestModel;
    std::vector<std::vector<int>> a;
    bestModel = arima->getARIMAModel(0, a, false, start, thread_num);
    //int p = bestModel[0];
    //((parameter*)thread_p) -> q = bestModel[1];
    //((parameter*)thread_p) -> aic = bestModel[2];
    //((parameter*)thread_p) -> predict = arima->predictValue(bestModel[0], bestModel[1]);

    for(int i = 0; i < thread_num - 1; i++)
    {
        pthread_join(thread_parameters[i].thread_id, NULL);

    }

    for(int i = 0; i < thread_num; i++)
    {
        std::cout<<thread_parameters[i].time<<" "<<thread_parameters[i].p<<" "<<thread_parameters[i].q<<" "<<thread_parameters[i].aic<<std::endl;
        
    }

    int best_p = bestModel[0];
    int best_q = bestModel[1];
    int best_predict = arima->predictValue(bestModel[0], bestModel[1]);
    int best_aic = bestModel[2];
    for(int i = 0; i < thread_num - 1; i++)
    {
        if(thread_parameters[i].aic < best_aic)
        {
            best_p = thread_parameters[i].p;
            best_q = thread_parameters[i].q;
            best_predict = thread_parameters[i].predict;
            best_aic = thread_parameters[i].aic;
        }
    }
   // int best_predict = arima->predictValue(best_p, best_q);


   // bestModel = arima->getARIMAModel(0, a, false);
    printf("BEST\tp:%d, q:%d\n", best_p, best_q);
    //predict = arima->predictValue(bestModel[0], bestModel[1]);
    //std::cout<<"aic : "<< aic<<std::endl;
    printf("Predict: %d, Expect: %lf\n", best_predict, expect);
}