#include <stdio.h>
#include "cachelab.h"

//reference https://blog.csdn.net/xbb224007/article/details/81103995?utm_source=blogxgwz0
int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,tmp,index,x,x1,x2,x3,x4,x5,x6,x7,x8,y;

   
    if(M==32){

        for(int i=0;i<M;i+=8){
            for(int j=0;j<N;j+=8){

                for(int x=i;x<i+8;x++){

                    for(int y = j;y<j+8;y++){

                        if(x==y){

                            tmp = A[x][y];
                            index = x; 
                        }
                        else{

                            B[y][x] = A[x][y];
                        }
                    }
                    if(i==j)
                        B[index][index] = tmp;
                }

            }
        }
    }

    else if(M==64){

        for(i=0;i<M;i+=8){
            for(j=0;j<N;j+=8){
                //visit the first 4 row
                //x mean xth row
                for(x=i;x<i+4;x++){
                    x1 = A[x][j];
                    x2 = A[x][j+1];
                    x3 = A[x][j+2];
                    x4 = A[x][j+3];
                    x5 = A[x][j+4];
                    x6 = A[x][j+5];
                    x7 = A[x][j+6];
                    x8 = A[x][j+7];

                    B[j][x] = x1;
                    B[j+1][x] = x2;
                    B[j+2][x] = x3;
                    B[j+3][x] = x4;

                    //last 4 columns of the first 4 row
                    //placed in B first 4 row's last columns temporarily
                    B[j][x+4] = x5;
                    B[j+1][x+4] = x6;
                    B[j+2][x+4] = x7;
                    B[j+3][x+4] = x8;
                }

                for(y=j;y<j+4;y++){

                    x1 = A[i+4][y];
                    x2 = A[i+5][y];
                    x3 = A[i+6][y];
                    x4 = A[i+7][y];
                    x5 = B[y][i+4];
                    x6 = B[y][i+5];
                    x7 = B[y][i+6];
                    x8 = B[y][i+7];

                    B[y][i+4] = x1;
                    B[y][i+5] = x2;
                    B[y][i+6] = x3;
                    B[y][i+7] = x4;
                    B[y+4][i] = x5;
                    B[y+4][i+1] = x6;
                    B[y+4][i+2] = x7;
                    B[y+4][i+3]=x8;
                }

                for(x = i+4;x<i+8;x++){

                    x1 = A[x][j+4];
                    x2 = A[x][j+5];
                    x3 = A[x][j+6];
                    x4 = A[x][j+7];

                    B[j+4][x] = x1;
                    B[j+5][x] = x2;

                    B[j+6][x] = x3;
                    B[j+7][x] = x4;

                }


            }
        }
    }

    else{

        for(int i=0;i<N;i+=16){
            for(int j=0;j<M;j+=16){
                for(int x=i;x<i+16&&x<N;x++){
                    for(int y = j;y<j+16&&y<M;y++){
                        if(x==y){
                            tmp = A[x][y];
                            index = x; 
                        }
                        else{
                            B[y][x] = A[x][y];
                        }
                    }
                    if(i==j)
                        B[index][index] = tmp;
                }

            }
        }   
    }

}

    /*
     * You can define additional transpose functions below. We've defined
     * a simple one below to help you get started.
     */

    /*
     * trans - A simple baseline transpose function, not optimized for the cache.
     */
    char trans_desc[] = "Simple row-wise scan transpose";
    void trans(int M, int N, int A[N][M], int B[M][N])
    {
        int i, j, tmp;

        for (i = 0; i < N; i++)
        {
            for (j = 0; j < M; j++)
            {
                tmp = A[i][j];
                B[j][i] = tmp;
            }
        }

    }

    /*
     * registerFunctions - This function registers your transpose
     *     functions with the driver.  At runtime, the driver will
     *     evaluate each of the registered functions and summarize their
     *     performance. This is a handy way to experiment with different
     *     transpose strategies.
     */
    void registerFunctions()
    {
        /* Register your solution function */
        registerTransFunction(transpose_submit, transpose_submit_desc);

        /* Register any additional transpose functions */
        registerTransFunction(trans, trans_desc);

    }

    /*
     * is_transpose - This helper function checks if B is the transpose of
     *     A. You can check the correctness of your transpose by calling
     *     it before returning from the transpose function.
     */
    int is_transpose(int M, int N, int A[N][M], int B[M][N])
    {
        int i, j;

        for (i = 0; i < N; i++)
        {
            for (j = 0; j < M; ++j)
            {
                if (A[i][j] != B[j][i])
                {
                    return 0;
                }
            }
        }
        return 1;
}