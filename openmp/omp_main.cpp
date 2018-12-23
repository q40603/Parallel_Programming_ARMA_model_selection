//
// Created by yue on 18-3-16.
//

#include <vector>
#include "cstdio"
#include "iostream"
#include "ARIMAModel.h"
#include <chrono>
#include <omp.h>
#define DATA_LIMIT 170
struct parameter{
    int p,q;
    double aic;
};

int main(int argc, char** argv){
    if(argc < 2){
        printf("Usage: %s <dataset>\n", argv[0]);
        return 0;
    }
    freopen(argv[1],"r",stdin);
    int data_count = 1;
    auto total_start = std::chrono::system_clock::now();
    double input;
    std::vector<double> data;
    while(std::cin>>input){
        data.push_back(input);
        if(++data_count > DATA_LIMIT) break;
    }
    data.pop_back();
    double minAIC = 1.7976931348623157E308D;
    std::vector<int> bestModel(3);
        int len = data.size();
        int size = ((len + 2) * (len + 1)) / 2 - 1;
        std::vector<std::vector<int>> model;
        model.resize(size);
        for(int i=0;i<size;i++) model[i].resize(size);
        int cnt = 0;
        int type = 0;
        for (int i = 0; i <= len; ++i)
        {
            for (int j = 0; j <= len - i; ++j)
            {
                if (i == 0 && j == 0)
                    continue;
                model[cnt][0] = i;
                model[cnt++][1] = j;
            }
        }
        parameter model_pool[cnt];
        std::vector<std::vector<double>> coe;
    #pragma omp parallel for private ( coe,type )
        for (int i = 0; i < cnt; ++i)
        {
        //std::cout << "; This thread ID is " << omp_get_thread_num() << std::endl;
            // 控制选择的参数
            //printf("=====%d, %d=====\n", model[i][0], model[i][1]);
            if (model[i][0] == 0)
            {
                MAMoel* ma = new MAMoel(data,model[i][1]);
                coe=ma->solveCoeOfMA();
                type = 1;
            }
            else if (model[i][1] == 0)
            {
                ARModel* ar = new ARModel(data, model[i][0]);
                coe=ar->solveCoeOfAR();
                type = 2;
            }
            else
            {
                ARMAModel* arma = new ARMAModel(data, model[i][0], model[i][1]);;
                coe=arma->solveCoeOfARMA();
                type = 3;
            }
            ARMAMath ar_math;
            model_pool[i].aic = ar_math.getModelAIC(coe, data, type);
            model_pool[i].p = model[i][0];
            model_pool[i].q = model[i][1];
            //std::cout<<aic<<std::endl;
            // 在求解过程中如果阶数选取过长，可能会出现NAN或者无穷大的情况
        }


        for (int i = 0; i < cnt; ++i){
            if (model_pool[i].aic<=1.7976931348623157E308D && !std::isnan(model_pool[i].aic) && model_pool[i].aic < minAIC)
            {
                minAIC = model_pool[i].aic;
               // std::cout<<aic<<std::endl;
                bestModel[0] = model_pool[i].p;
                bestModel[1] = model_pool[i].q;
                bestModel[2] = (int)std::round(minAIC);
                //printf("%d, %d, %d\n", bestModel[0], bestModel[1], predictValue(bestModel[0], bestModel[1]));
            }            
        }
    printf("Best model:\n");
    printf("p: %d,\tq: %d\n", bestModel[0], bestModel[1]);
    auto total_end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = total_end - total_start;
    printf("Total time: %lf\n", elapsed_seconds.count());


    /*for(int p = 0 ; p < len ; p++){
        for(int q = 1 ; q <  ; q++){
            ARMAModel* arma = new ARMAModel(data, p, q);
            std::vector<std::vector<double>> coe;
            coe=arma->solveCoeOfARMA();
            ARMAMath ar_math;
            double aic = ar_math.getModelAIC(coe, data, 3); 
            std::cout<<p<<" "<<q<<" aic : "<< aic<<std::endl;     
        }
    }*/


}