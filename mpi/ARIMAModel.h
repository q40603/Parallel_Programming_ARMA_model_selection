//
// Created by yue on 18-3-16.
//

#ifndef ARIMAMODEL_H
#define ARIMAMODEL_H
#include <vector>
#include "ARModel.h"
#include "MAModel.h"
#include "ARMAModel.h"
#include <cmath>
class ARIMAModel{
private:
    std::vector<double> dataArray;
    std::vector<std::vector<double>> arima;

public:
    ARIMAModel(std::vector<double> dataArray) {this->dataArray.assign(dataArray.begin(),dataArray.end());}

    std::vector<int> getARIMAModel(int start, int thread_num){
        std::vector<double> data = dataArray;

        double minAIC = 1.7976931348623157E308D;
        std::vector<int> bestModel(3);
        int type = 0;
        std::vector<std::vector<double>> coe;

        // model产生, 即产生相应的p, q参数
        int len = data.size();

        int size = ((len + 2) * (len + 1)) / 2 - 1;
        std::vector<std::vector<int>> model;
        model.resize(size);
        for(int i=0;i<size;i++) model[i].resize(size);

        int cnt = 0;
        for (int i = start; i <= len; i += thread_num)
        {
            for (int j = 0; j <= len - i; ++j)
            {
                if (i == 0 && j == 0)
                    continue;
                model[cnt][0] = i;
                model[cnt++][1] = j;
            }
        }
        
        //std::cout<<cnt<<std::endl;
        //std::cout<<size<<std::endl;
        for (int i = 0; i < cnt; ++i)
        {
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
            double aic = ar_math.getModelAIC(coe, data, type);
            //std::cout<<aic<<std::endl;
            // 在求解过程中如果阶数选取过长，可能会出现NAN或者无穷大的情况
           
            if (aic<=1.7976931348623157E308D && !std::isnan(aic) && aic < minAIC)
            {
                minAIC = aic;
               // std::cout<<aic<<std::endl;
                bestModel[0] = model[i][0];
                bestModel[1] = model[i][1];
                bestModel[2] = (int)std::round(minAIC);
                this->arima = coe;
                //printf("%d, %d, %d\n", bestModel[0], bestModel[1], predictValue(bestModel[0], bestModel[1]));
            }
        }
        return bestModel;
    }
    
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


    int predictValue(int p, int q){
        std::vector<double> data = dataArray;
        int n = data.size();
        int predict = 0;
        double tmpAR = 0.0, tmpMA = 0.0;
        std::vector<double> errData(q + 1);
        if (p == 0)
        {
            std::vector<double> maCoe(this->arima[0]);
            for(int k = q; k < n; ++k)
            {
                tmpMA = 0;
                for(int i = 1; i <= q; ++i)
                {
                    tmpMA += maCoe[i] * errData[i];
                }
                //产生各个时刻的噪声
                for(int j = q; j > 0; --j)
                {
                    errData[j] = errData[j - 1];
                }
                errData[0] = gaussrand()*std::sqrt(maCoe[0]);
            }

            predict = (int)(tmpMA); //产生预测
        }
        else if (q == 0)
        {
            std::vector<double> arCoe(this->arima[0]);

            for(int k = p; k < n; ++k)
            {
                tmpAR = 0;
                for(int i = 0; i < p; ++i)
                {
                    tmpAR += arCoe[i] * data[k - i - 1];
                }
            }
            predict = (int)(tmpAR);
        }
        else
        {
            std::vector<double> arCoe(this->arima[0]);
            std::vector<double> maCoe(this->arima[1]);
            for(int k = p; k < n; ++k)
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

                //产生各个时刻的噪声
                for(int j = q; j > 0; --j)
                {
                    errData[j] = errData[j-1];
                }

                errData[0] = gaussrand() * std::sqrt(maCoe[0]);
            }

            predict = (int)(tmpAR + tmpMA);
        }
        return predict;
    }

};
#endif //ARIMAMODEL_H
