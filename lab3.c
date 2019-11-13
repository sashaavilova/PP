#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define m 8
#define n 7
#define count (n / 3 * m)

/*20 Включить в новую матрицу каждый третий столбец старой матрицы.*/

void init(int *matrix)
{    
     for (int i = 0; i < m*n; i++)
         matrix[i] = 1+i;
}

void print(int *source, int nn, int mm){

    for (int i = 0; i < mm; i++){
        for (int j = 0; j < nn; j++)
            printf("%i\t", source[i * n + j]);
        printf("\n");
        }
}

int main(int argc, char *argv[])
{
    int i, j, msize = m * n;
    int disp[count],
        len[count],
        *source = (int *)malloc(msize * sizeof(int)),
        *result = (int *)malloc(msize * sizeof(int)),
        *result_null = (int *)malloc(msize * sizeof(int)),
        *rec;
    
   for (i = 0; i < count; i++)
    {
        len[i] = 1;
    }
   
    // конструирование типа для описания в строке каждого 3его элемента 
    int k = 2, l = 0, cols = n / 3;
    
   for (i = 0; i < m; ++i){
      for (j = 0; j < cols; ++j){
            disp[l++] = k;
            k += 3;
        }
        k += n % 3;
    }

   rec = (int *)malloc(msize * sizeof(int));

    int size, rank;
    
    MPI_Status status;    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Datatype newtype;
    
    if(size!=2){
        printf("Введите 2 процесса");
        return 0;
    }
    // count-количество блоков; *len- количество элементов в каждом блоке; *disp-расстояние между блоками, передоваемые данные
    MPI_Type_indexed(count, len, disp, MPI_INTEGER, &newtype);
    MPI_Type_commit(&newtype);
    
    if (rank == 0)
    {
        printf("\nRunning %d of threads", size);
        printf("\nUsing matrix %dx%d", m, n);
        init(source);
        printf("\nSource matrix: \n");
        print(source, n, m);
        MPI_Send(source, 1, newtype, 1, 1, MPI_COMM_WORLD);
///////////////////////////
        MPI_Recv(rec, 1, newtype, 1, 1, MPI_COMM_WORLD, &status);
        printf("\nRunning %d of rang", rank);
        printf("\nNULL matrix: \n");
        print(rec, n, m);
   
    }

    if (rank != 0)
    {
        MPI_Recv(result, 1, newtype, 0, 1, MPI_COMM_WORLD, &status);
        printf("\nResult matrix: \n");
        print(result, n, m);

         MPI_Send(result, 1, newtype, 0, 1, MPI_COMM_WORLD);
    }

       


    free(result_null);
    free(source);
    free(result);
    MPI_Type_free(&newtype);
    MPI_Finalize();
    return 0;
}
