 %module k8000
    %{
    /* Includes the header in the wrapper code */
    #include "k8000.h"
    %}
   
    /* Parse the header file to generate wrappers */
    %include "k8000.h"

    /* add wrapper functions for the global arrays */
    %inline %{
    short IOconfig_matrix_getitem(short data[MaxChains][MaxIOchip+1], int i,int j) {
        return data[i][j];
    }
    short IOdata_matrix_getitem(short data[MaxChains][MaxIOchip+1], int i,int j) {
        return data[i][j];
    }
    short IO_matrix_getitem(short data[MaxChains][MaxIOchannel+1], int i,int j) {
        return data[i][j];
    }
    short DAC_matrix_getitem(short data[MaxChains][MaxDAchannel+1], int i,int j) {
        return data[i][j];
    }
    short AD_matrix_getitem(short data[MaxChains][MaxADchannel+1], int i,int j) {
        return data[i][j];
    }
    short DA_matrix_getitem(short data[MaxChains][MaxDAchannel+1], int i,int j) {
        return data[i][j];
    }
    %}
