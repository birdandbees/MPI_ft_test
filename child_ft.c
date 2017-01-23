
#include <stdio.h>
#include "mpi.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sig_handler(int signo)
{
  if (signo == SIGINT)
  {
    printf("received SIGINT\n");
  }
}

int main( int argc, char *argv[] )
{
       if (signal(SIGINT, sig_handler) == SIG_ERR)
          printf("\ncan't catch SIGINT\n");
	MPI_Comm intercomm;
        int echo = 1;
	int err, rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_get_parent(&intercomm);
	if ( intercomm == MPI_COMM_NULL ) printf("error, no parent\n");
        //sleep(8);

	if (rank <= 2 ){
		printf("child %d: sending %d\n", rank, echo);
		err = MPI_Send(&echo, 1, MPI_INT, 0, 0, intercomm);
		fflush(stdout);
	}else { 
                //sleep(15);
		printf("child %d: is failing, exiting\n", rank);
		err = MPI_Send(&echo, 1, MPI_INT, 0, 0, intercomm);
		fflush(stdout);
		MPI_Finalize();
		return 2;;
	};
	MPI_Finalize();
	return 0;
}
