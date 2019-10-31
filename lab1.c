#include "mpi.h"
#include <stdio.h>
#include <math.h>


static double f(double a, double c);
static double fi(double a, double c);

void main(int argc, char* argv[])
{
	int done = 0, n=0, myid, numprocs, i;
	double myfunk, funk, h, sum, x;
	double  xl = -0.2,	// low  border
			xh = 1.5,	// high border
			xc = 1.7;

	double startwtime, endwtime;
	int  namelen, pos;
	char buffer[128];
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Status stats;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Get_processor_name(processor_name, &namelen);

	fprintf(stderr, "Process %d on %s\n",myid, processor_name);
	fflush(stderr);
	
	
		if (myid == 0)
		{
			printf("xl xh xc "); fflush(stdout);
			scanf("%lf %lf %lf", &xl, &xh, &xc); 

			/*
			for (i = 1; i < numprocs; i++)
				MPI_Send(&n,1,MPI_INT,i,1,MPI_COMM_WORLD);    */
			
			pos = 0;
			
			MPI_Pack(&xl, 1, MPI_DOUBLE, buffer, 128, &pos, MPI_COMM_WORLD);
			MPI_Pack(&xh, 1, MPI_DOUBLE, buffer, 128, &pos, MPI_COMM_WORLD);
			MPI_Pack(&xc, 1, MPI_DOUBLE, buffer, 128, &pos, MPI_COMM_WORLD);
				
			/*startwtime = MPI_Wtime();*/
			
		//	MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD );// two Bcast, root process, send
			
		}
		
		MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD );// one Bcast, all process
		
		if (myid != 0)
		{
		//MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD );// two Bcast, root process, send
			pos = 0;
			
			MPI_Unpack(buffer, 128, &pos, &xl, 1, MPI_DOUBLE, MPI_COMM_WORLD);
			MPI_Unpack(buffer, 128, &pos, &xh, 1, MPI_DOUBLE, MPI_COMM_WORLD);
			MPI_Unpack(buffer, 128, &pos, &xc, 1, MPI_DOUBLE, MPI_COMM_WORLD);

		}
		
		/*else
			MPI_Recv(&n,1,MPI_INT,0,1,MPI_COMM_WORLD,&stats);*/
		
		
			
	while (!done)
		{
			if(myid == 0)
			{
				printf(" n ");
				fflush(stdout);
				scanf("%d", &n);
				
				if(n!=0)
				startwtime = MPI_Wtime();
			}
			 			
			MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD );// one Bcast, all process
			printf("\n process = %d , n = %d \n", myid, n);
			if (n == 0)	done = 1;
			else
			{
				h = (xh - xl) / (double)n;
				sum = 0.0;
				for (i = myid + 1; i <= n; i += numprocs)
				{
					x = xl + h * ((double)i - 0.5);
					sum += f(x, xc);
				}
				myfunk = h * sum;
				printf("Process %d SUMM %.16f\n", myid, myfunk);

				/* Sending the local sum to node 0 */
			/*	if (myid != 0)
					MPI_Send(&myfunk,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD); */
			
				MPI_Reduce(&myfunk, &funk, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
				

				if (myid == 0)
				{

					/*funk = myfunk;
					for (i = 1; i < numprocs; i++)
					{
						MPI_Recv(&myfunk,1,MPI_DOUBLE,i,1,MPI_COMM_WORLD,&stats);
						funk += myfunk;
					}*/

					printf("Integral is approximately  %.16f, Error   %.16f\n",funk, funk - fi(xh, xc) + fi(xl, xc));
					endwtime = MPI_Wtime();
					printf("Time of calculation = %f\n", endwtime - startwtime);
				}
			}
		}
	MPI_Finalize();
}


static double f(double a, double c)
{
	return cos(c * a);
}

static double fi(double a, double c)
{
	return  sin(c * a) / c;
}

