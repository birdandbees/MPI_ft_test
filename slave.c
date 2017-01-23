/*
∗ Simulator for fault−tolerance
∗ A number o f random processes fail during computation
∗ This is the master/slave−version with fault−tolerance
∗ Author : Knut Imar Hagen
*/
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <time.h>
# include <errno.h>
# include "mpi.h"
# define IC_TAG 1
int totalsize = 2000; // total size of the square matrix , length & width
int main(int argc , char ** argv ) {
  int i , j , slaveid , origsize , currsize ;
  int procfail, numloop , *failingprocs, ierr, flag , universe;
  double *submat , val ;
  void ** universeattr;
  MPI_Init(&argc, &argv) ;
  MPI_Comm_size (MPI_COMM_WORLD, &origsize) ;
  currsize = origsize ;
  MPI_Comm master_comm ;
  MPI_Status *status ;
  MPI_Comm_get_parent(&master_comm ) ;
  if ( master_comm == MPI_COMM_NULL)
    printf( "NULL − COMMUNICATOR" ) ;
  MPI_Recv(&slaveid , 1 , MPI_INT , 0 , IC_TAG , master_comm , MPI_STATUS_IGNORE ) ;
  MPI_Recv(&numloop , 1 , MPI_INT , 0 , IC_TAG , master_comm , MPI_STATUS_IGNORE ) ;
  MPI_Recv(&procfail, 1 , MPI_INT , 0 , IC_TAG , master_comm , MPI_STATUS_IGNORE ) ;
  failingprocs = malloc(( unsigned ) sizeof(int) *procfail) ;
  memset (failingprocs , 0 , sizeof (int) *procfail) ;
  MPI_Recv (failingprocs , procfail, MPI_INT , 0 , IC_TAG , master_comm , MPI_STATUS_IGNORE) ;
  printf("receiving %d, %d, %d\n", slaveid, numloop, procfail);
  // Allocate the submatrix
  submat = malloc (sizeof ( double ) * totalsize * totalsize/origsize) ;
  memset(submat , 0 , sizeof(double) * totalsize * totalsize/origsize) ;
  /* Initialize the submatrix
  ∗ the first element will always be rank number + 1
  */
  for ( i = 0 ; i < totalsize/origsize; i++) {
    for( j = 0 ; j < totalsize; j++) {
      submat[i*totalsize+j] = (slaveid+1) * ( j + i + 1 ) ;
    }
  }
  /* If the number of loops is set higher than 0 , the computation will run
  ∗ the give n number of times , and the processes listed to be failed
  ∗ will fail in a sequentially order in a regular manner
  */
 if ( numloop > 0 ) {
    int countfail = 0;
    int upcountfail = 0;
    if ( procfail == 0 )
      procfail = 1;
    else
  upcountfail = numloop/procfail ;
  for ( i = 0 ; i <numloop ; i ++) {
    if ( numloop/procfail == upcountfail) {
      if ( countfail < procfail && failingprocs[countfail++] == slaveid ) {
        printf(" I am slave %d and I am failing \n " , slaveid) ;
        fflush(stdout) ;
        exit(10);
       }
      upcountfail = 0;
     }
    upcountfail ++;
    for ( j = 0 ; j <(totalsize* totalsize/origsize)/2 ; j ++) {
      double swp = submat[j] ;
      submat[j] = submat[totalsize * totalsize/origsize - j];
      submat[totalsize * totalsize/origsize - j] = swp;
    }
    for ( j = 0 ; j <(totalsize * totalsize/origsize) / 2 ; j ++) {
      double swp = submat[totalsize * totalsize/origsize - j];
      submat[totalsize * totalsize/origsize - j] = submat[j];
      submat[j] = swp ;
   }
  }
  }
  // Processes that have not failed will say that they are alive
  printf( " S l a v e #%d : submat [0]=% f \n " , slaveid , submat[0]) ;
  MPI_Send(submat, 1 , MPI_DOUBLE, 0 , IC_TAG, master_comm) ;
  fflush(stdout) ;
  MPI_Finalize( ) ;
  exit(0);
}
