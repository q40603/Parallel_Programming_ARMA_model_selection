//
// Created by yue on 18-3-16.
//
#include "mpi.h"
#include <vector>
#include "cstdio"
#include "iostream"
#include "ARIMAModel.h"
#include <chrono>

#define DATA_LIMIT 100

int main(int argc, char** argv){
    if(argc < 2){
        printf("Usage: %s <dataset>\n", argv[0]);
        return 0;
    }
    freopen(argv[1],"r",stdin);

    auto total_start = std::chrono::system_clock::now();

    /*****  MPI Var  *****/
    int rank, processes;
    int dest = 0, tag =0;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

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
    std::vector<int> bestModel = arima.getARIMAModel(rank, processes);
    int bestPredict = arima.predictValue(bestModel[0], bestModel[1]);

    auto cpt_end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = cpt_end-cpt_start;
    printf("%d Computation time: %lf\n", rank, elapsed_seconds.count());

    /*****  IPC  *****/
    if(rank == 0){
        int recv[4]; // bestModel from other unit
        for(int i = 1; i<processes; i++){
            MPI_Recv(recv, 4, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
            //printf("Recv: p(%d), q(%d), aic(%d), predict(%d)\n", recv[0], recv[1], recv[2], recv[3]);
            if(recv[2] < bestModel[2]){
                //printf("new best! from %d\n", i);
                bestModel[0] = recv[0];
                bestModel[1] = recv[1];
                bestModel[2] = recv[2];
                bestPredict = recv[3];
            }
        }
    }else{
        int source[4] = {bestModel[0], bestModel[1], bestModel[2], bestPredict};
        MPI_Send(source, 4, MPI_INT, dest, tag, MPI_COMM_WORLD);
        //printf("%d Send: p(%d), q(%d), aic(%d), predict(%d)\n", rank, bestModel[0], bestModel[1], bestModel[2], bestPredict);
        auto total_end = std::chrono::system_clock::now();
        elapsed_seconds = total_end - total_start;
        printf("%d Total time: %lf\n", rank, elapsed_seconds.count());
        MPI_Finalize();
        return 0;
    }

    /***** Result *****/    
    printf("Best model:\n");
    printf("p: %d,\tq: %d\n", bestModel[0], bestModel[1]);
    printf("Predict: %d,\tExpect: %lf\n", bestPredict, expect);
    auto total_end = std::chrono::system_clock::now();
    elapsed_seconds = total_end - total_start;
    printf("Total time: %lf\n", elapsed_seconds.count());
    MPI_Finalize();
}