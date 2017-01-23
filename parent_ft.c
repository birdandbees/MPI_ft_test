#include <stdio.h>
#include "mpi.h"
#include <unistd.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
		int response[4] = {0,0,0,0};;
		int err=0, errcodes[256], rank;
		MPI_Comm intercomm;

		MPI_Init(&argc, &argv);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		MPI_Errhandler_set(MPI_COMM_WORLD,MPI_ERRORS_RETURN); 
		MPI_Errhandler_set(MPI_COMM_SELF,MPI_ERRORS_RETURN); 
		if (rank != 0) { MPI_Finalize();return 0; };
		int num_of_children = 4;
		printf("parent: my rank is %d\n", rank);
		err = MPI_Comm_spawn("child_ft", MPI_ARGV_NULL, num_of_children,
						MPI_INFO_NULL, 0, MPI_COMM_SELF,
						&intercomm, errcodes);  
		if (err) printf("parent: Error in MPI_Comm_spawn\n");
		MPI_Request request[4];
		MPI_Status status[4];
		int flag[4];
		sleep(5);
		for (int i = 0; i < num_of_children; i++)
		{
				err = MPI_Irecv(&response[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, intercomm, &request[i]);
				if (err != MPI_SUCCESS)
				{
						printf("parent: receiving failed from one of children- %d\n", i);

				}else{
						flag[i] = 0;
				}
				//sleep(15);        
		}
		// test if requests are complete for serveral times
		// quit waiting after 3 tries
		int try = 0;
		while( try <= 3 )
		{
				for (int i = 0; i < 4; i++)
				{
						MPI_Test(&request[i], &flag[i], &status[i]);

				};
				try++;
		}
		sleep(2);

		for (int i = 0; i < num_of_children; i++)
		{
				printf("buffer %d is %d\n", i, response[i]);
		};

		MPI_Finalize();

		return 0;
}

