#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/wait.h>
/*Вычислить Y[i] = a * X[i] + Y[i]  (SAXPY) для массива случайных чисел. Определить максимальное и минимальное значения полученного массива*/

//Функция для заполнения размера блоков и индексов
void Construct(int processorCount, int arraySize, int* sendcounts, int* displs)
{
	int length = arraySize / processorCount;
	int i;

	for (i = 0; i < processorCount - 1; i++)
	{
		sendcounts[i] = length;
		displs[i] = length * i;
	}
	sendcounts[i] = length + (arraySize % processorCount);
	displs[i] = length * i;
}

void SAXPY(double* x, double* y, double* z, int n){
	
	for(int i=0; i<n; i++)
		z[i]= 2* x[i] + y[i];
}

double min_mas(double* z, int n){
	double min=z[0];
	for(int i=1; i<n; i++){
		if(z[i]<min)
			min=z[i];	
	}
	return min;
}

double max_mas(double* z, int n){
	double max=z[0];
	for(int i=1; i<n; i++){
		if(z[i]>max)
			max=z[i];	
	}
	return max;
}

int main(int argc, char** argv)
{
	int processorRank, processorCount;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&processorCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &processorRank);
	
	double min, min_, max, max_;
	double *x, *y, *buf_x, *buf_y, *z, * buf_z, *z_mpi;
	
	int *lengthSend, *displs;
	double *min_buf, *max_buf;
	
	int n = atoi(argv[1]), n_copy=n;

	lengthSend = (int*)malloc(processorCount * sizeof(int));
	displs = (int*)malloc(processorCount * sizeof(int));
		
	min_buf = (double *)malloc(processorCount * sizeof(double));
	max_buf = (double *)malloc(processorCount * sizeof(double));
	
	if (processorRank == 0)
	{
//Выделение памяти под массив
		
		x = (double *)malloc(n_copy * sizeof(double));
		y = (double *)malloc(n_copy * sizeof(double));
		z = (double *)malloc(n_copy * sizeof(double));
		
		z_mpi = (double *)malloc(n_copy * sizeof(double));
				
//Инициализация массива
		for (int i = 0; i < n_copy; i++){
			x[i] = (double)rand() / RAND_MAX *(double)(rand() % 100);
			y[i] = (double)rand() / RAND_MAX *(double)(rand() % 100);
		}
		
//вывод матрицы заполненой первоначально
		printf(" \n X ");
		for (int i = 0; i < n; i++)
			printf(" % 4.6f ", x[i]);

		printf(" \n Y ");
		for (int i = 0; i < n; i++)
			printf(" % 4.6f ", y[i]);
		
		SAXPY(x,y,z,n);
		min = min_mas(z,n);
		max = max_mas(z,n);
		
		//Вывод результата после последовательной обработки
		printf(" \n Z ");
		for (int i = 0; i < n; i++)
			printf(" % 4.6f ", z[i]);
		printf("\n MIN =  % 4.6f , MAX = % 4.6f", min, max);
		
		
		//заполнение массивов для отправки длин порций
		Construct(processorCount, n, lengthSend, displs);
	}
	
		buf_x = (double *)malloc(n * sizeof(double));
		buf_y = (double *)malloc(n * sizeof(double));
		buf_z = (double *)malloc(n * sizeof(double));

//Отправить всем процессам длину массивов которые нужно будет
	//принять
	MPI_Scatter(lengthSend, 1, MPI_INT, &n, 1, MPI_INT, 0,MPI_COMM_WORLD);

//Отправка порций массивов для вычислений и вывод
	MPI_Scatterv(x, lengthSend, displs, MPI_DOUBLE, buf_x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatterv(y, lengthSend, displs, MPI_DOUBLE, buf_y, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	printf(" \nRank % d receive % d:" , processorRank, n);
	printf(" \nbuf_x \n " );
	for (int i = 0; i < n; ++i)
		printf(" % 4.6f ", buf_x[i]);
	
	printf(" \nbuf_y \n ");
	for (int i = 0; i < n; ++i)
		printf(" % 4.6f ", buf_y[i]);
	
	//Вывод результата работы процесса
	SAXPY(buf_x,buf_y,buf_z,n);
	min_= min_mas(buf_z,n);
	max_ = max_mas(buf_z,n);
	
	printf(" \nbuf_z \n ");
	for (int i = 0; i < n; ++i)
		printf(" % 4.6f ", buf_z[i]);
	
	printf(" \nProcess % d result min = % lf,  max =  % lf\n ",  processorRank, min_, max_);

//Сборка результатов
	MPI_Gather(&min_, 1, MPI_DOUBLE, min_buf, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Gather(&max_, 1, MPI_DOUBLE, max_buf, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Gatherv(buf_z, n, MPI_DOUBLE, z_mpi, lengthSend, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	free(buf_z); free(buf_x); free(buf_y);
	
	MPI_Finalize();

//Вывод результата параллельной обработки и освобождение памяти
	if (processorRank == 0)
	{
		printf("\nZ: \n");
		for (int i = 0; i < n_copy;i++)		
			printf(" % 4.6lf ", z_mpi[i]);
		
	min_=min_mas(min_buf,processorCount);
	max_=max_mas(max_buf,processorCount);
		
		
	printf("\n MIN = % 4.6lf , MAX = % 4.6lf \n", min_, max_);

	free(displs); free(lengthSend); free(x); free(y); free(z); free(min_buf); free(z_mpi); free(max_buf);		
	}

	return 0;
}
