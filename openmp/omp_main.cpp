#include <vector>
#include "cstdio"
#include "iostream"
#include "ARIMAModel.h"
#include "ARMAMath.h"
#include <chrono>
#include <omp.h>
#include <cmath>
#pragma optimize level=3
#pragma GCC optimize("-O3")
//#define DATA_LIMIT 170
int DATA_LIMIT;
struct parameter{
    int p,q;
    double aic;
    std::vector<std::vector<double>> coe;
};
double gaussrand()
{
    static double V1, V2, S;
    static int phase = 0;
    double X;
    srand(time(NULL));
    if ( phase == 0 ) {
        do {
            double U1 = (double)rand() / RAND_MAX;
            double U2 = (double)rand() / RAND_MAX;
            V1 = 2 * U1 - 1;
            V2 = 2 * U2 - 1;
            S = V1 * V1 + V2 * V2;
        } while(S >= 1 || S == 0);
        X = V1 * sqrt(-2 * log(S) / S);
    } else
        X = V2 * sqrt(-2 * log(S) / S);
    phase = 1 - phase;
    return X;
}
int main(int argc, char** argv){
    if(argc < 3){
        printf("Usage: %s <dataset>\n", argv[0]);
        DATA_LIMIT = atoi(argv[1]);
        return 0;
    }
    freopen(argv[1],"r",stdin);
    int data_count = 1;
    auto total_start = std::chrono::system_clock::now();
    double input,prev;
    std::vector<double> data;
    while(std::cin>>input){
        if(data_count!=1)
            data.push_back(input-prev);
        if(++data_count > DATA_LIMIT) break;
        prev = input;
    }
    double expect =  data.back();
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
    #pragma omp parallel for private ( coe,type ) schedule( static, 1 )
        for (int i = 0; i < cnt; ++i)
        {
        //std::cout << "; This thread ID is " << omp_get_thread_num() << std::endl;
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
            model_pool[i].coe = coe;
            //std::cout<<aic<<std::endl;
        }

        std::vector<std::vector<double>> best_coe;
        int best_record = 0;
    #pragma omp parallel for shared(minAIC,best_record) schedule( static, 1 )
        for (int i = 0; i < cnt; ++i){
            if (model_pool[i].aic<=1.7976931348623157E308D && !std::isnan(model_pool[i].aic) && model_pool[i].aic < minAIC)
            {
            //#pragma omp critical    
                best_record = i;
                minAIC = model_pool[i].aic;
            }
        }
               bestModel[0] = model_pool[best_record].p;
                bestModel[1] = model_pool[best_record].q;
                bestModel[2] = (int)std::round(model_pool[best_record].aic);
                best_coe = model_pool[best_record].coe;
//------------------  predict  ------------------------------------------------
        double predict = 0.0;
        double tmpAR = 0.0, tmpMA = 0.0;
        std::vector<double> errData(bestModel[1] + 1);
        int p = bestModel[0], q = bestModel[1];
        if (p == 0)
        {
            std::vector<double> maCoe(best_coe[0]);
            for(int k = q; k < len; ++k)
            {
                tmpMA = 0;
                for(int i = 1; i <= q; ++i)
                {
                    tmpMA += maCoe[i] * errData[i];
                }
                //white noise
                for(int j = q; j > 0; --j)
                {
                    errData[j] = errData[j - 1];
                }
                errData[0] = gaussrand()*std::sqrt(maCoe[0]);
            }

            predict = tmpMA; //predict
        }
        else if (q == 0)
        {
            std::vector<double> arCoe(best_coe[0]);
            for(int k = p; k < len; ++k)
            {
                tmpAR = 0;
                for(int i = 0; i < p; ++i)
                {
                    tmpAR += arCoe[i] * data[k - i - 1];
                }
            }
            predict = tmpAR;
        }
        else
        {
            std::vector<double> arCoe(best_coe[0]);
            std::vector<double> maCoe(best_coe[1]);
            for(int k = p; k < len; ++k)
            {
                tmpAR = 0;
                tmpMA = 0;
                for(int i = 0; i < p; ++i)
                {
                    tmpAR += arCoe[i] * data[k- i - 1];
                }
                for(int i = 1; i <= q; ++i)
                {
                    tmpMA += maCoe[i] * errData[i];
                }

                //white noise
                for(int j = q; j > 0; --j)
                {
                    errData[j] = errData[j-1];
                }

                errData[0] = gaussrand() * std::sqrt(maCoe[0]);
            }

            predict = tmpAR + tmpMA;
        }


//-----------------------------------------------------------------------------



    printf("Best model:\n");
    printf("p: %d,\tq: %d\n", bestModel[0], bestModel[1]);
    printf("Predict: %lf,\tExpect: %lf\n", predict + prev, input);
    auto total_end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = total_end - total_start;
    printf("Total time: %lf\n", elapsed_seconds.count());


}