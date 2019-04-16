#include "sum.h"

void omp_sum(double *sum_ret)
{
    double sum = 0;
    #pragma omp parallel
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) {
            sum += x[i * nt + tid];
        } 
    }
    *sum_ret = sum;
}

void omp_critical_sum(double *sum_ret)
{
    double sum = 0;
    #pragma omp parallel
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) 
        {
            #pragma omp critical
            sum += x[i * nt + tid];
        } 
    }
    *sum_ret = sum;
}

void omp_atomic_sum(double *sum_ret)
{
    double sum = 0;
    #pragma omp parallel
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) 
        {
            #pragma omp atomic
            sum += x[i * nt + tid];
        } 
    }
    *sum_ret = sum;
}

void omp_local_sum(double *sum_ret)
{
    int MAX_THREADS = omp_get_max_threads();
    double sum[MAX_THREADS];
    int i;
    for( i = 0; i < MAX_THREADS; i++ ) {
        sum[i] = 0.;
    }
    #pragma omp parallel
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) 
        {
            sum[tid] += x[i * nt + tid];
        } 
    }
    *sum_ret = 0;
    for( i = 0; i < MAX_THREADS; i++ ) {
        *sum_ret = *sum_ret + sum[i];
    }

}

void omp_padded_sum(double *sum_ret)
{
    int PADDING = 8;
    int MAX_THREADS = omp_get_max_threads();
    double sum[MAX_THREADS * PADDING];
    int i;
    for( i = 0; i < MAX_THREADS; i++ ) {
        sum[i * PADDING] = 0.;
    }
    #pragma omp parallel
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) 
        {
            sum[tid * PADDING] += x[i * nt + tid];
        } 
    }
    *sum_ret = 0;
    for( i = 0; i < MAX_THREADS; i++ ) {
        *sum_ret = *sum_ret + sum[i * PADDING];
    }
}

void omp_private_sum(double *sum_ret)
{
    *sum_ret = 0.;
    double sum = 0.;
    #pragma omp parallel private(sum)
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) 
        {
            sum += x[i * nt + tid];
        } 
        #pragma omp critical
        *sum_ret += sum;
    }
}

void omp_reduction_sum(double *sum_ret)
{
    double sum = 0;
    #pragma omp parallel reduction (+:sum)
    {
        int i;
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        for( i = 0; i * nt +  tid < size; i++ ) 
        {
            sum += x[i * nt + tid];
        } 
    }
    *sum_ret = sum;

}
