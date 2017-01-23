/*
 * Simulator for fault−tolerance
* A number o f random processes fail during computation
* This is the master/slave−version with fault−tolerance
* Author : Knut Imar Hagen
*/
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <time.h>
# include "mpi.h"
# define IC_TAG 1
static MPI_Comm *slave_comm ;
static MPI_Request *recvrequest ;
static MPI_Status *status ;
static int totalsize = 2000; //total size of the square matrix , length & width
int help(int rank ) {
  if ( rank == 0 ) {
    printf( " usage : simulator <procfail> <numloops>\n " ) ;
    printf( " procfail − Number of randomly failing " ) ;
    printf( " processes during the computation\n " ) ;
    printf( " numloops − Number of loops of computation , " ) ;
    printf( " to adjust computation time\n " ) ;
    printf( " NOTE: procfail must be less or equal to numloops\n " ) ;
  }
  MPI_Finalize ( ) ;
  exit (0);
}

int main(int argc , char ** argv ) {
  int i, j, rank, icrank, icsize, origsize;
  int procfail, numloop, *failingprocs, ierr, flag, universe;
  double *submat, *recvval;
  void ** universeattr;
  char filename[100] ;
  FILE *fp;
  MPI_Info simulatorinfo;
  MPI_Init (&argc , &argv ) ;
  MPI_Comm_rank (MPI_COMM_WORLD, &rank) ;
  MPI_Comm_size (MPI_COMM_WORLD, &origsize) ;
  printf("my rank is %d\n", rank);
  // If there are not 2 arguments , print help and quit
  if (argc != 3)
    help(rank) ;
  procfail = atoi(argv[1]);
  numloop = atoi(argv[2]) ;
  // number of failing processes must be less or equal to number of loops
  if (procfail > numloop || procfail < 0 || numloop < 0 ) {
    help(rank) ;
  }
/*If the simulator shall fail some processes , let rank 0 make a random
* list of the failing processes and send the list to every process .
* Rank 0 will not fail , because this will be the manager process
*/
  if( procfail > 0) {
    failingprocs = malloc(sizeof(int) * procfail) ;
    memset(failingprocs, 0 , sizeof(int) * procfail) ;
  if( rank == 0 ) {
    srand (time(NULL )) ;
    int k ;
    for ( k = 0; k< procfail; k++) {
      int tmp = 0 ;
      int ok = 1 ;
      do {
          ok = 1 ;
          tmp = rand ()%origsize ;
          int l ;
          for ( l = 0 ; l <k ; l ++) {
            if (failingprocs[l] == tmp )
              ok = 0 ;
          }
          if ( tmp == 0 )
            ok = 0 ;
      } while(!ok) ;
    failingprocs[k] = tmp ;
   }
  }
// MPI_Bcast(failingprocs, procfail, MPI_INT, 0 ,MPI_COMM_WORLD ) ;
  }
// Getting the universe size
  MPI_Attr_get (MPI_COMM_WORLD, MPI_UNIVERSE_SIZE , &universeattr , &flag) ;
  if ( flag ==1)
  universe = (int) * universeattr ;
  printf("universe is %d\n", universe);
  // Initializing communicators , requests and receive values
  slave_comm = (MPI_Comm * ) malloc(( unsigned )(universe * sizeof(MPI_Comm))) ;
  recvrequest = (MPI_Request * ) malloc (( unsigned )(universe * sizeof (MPI_Comm))) ;
  recvval = (double * ) malloc((unsigned) (universe * sizeof (double))) ;
  for ( i = 0 ; i < universe;++ i ) {
    slave_comm[i] = MPI_COMM_NULL;
    recvrequest[i] = MPI_REQUEST_NULL ;
  }
  // Spawn the slaves
  sprintf(filename , "./simulator_fault_schema" ) ;
  MPI_Info_create(&simulatorinfo) ;
  MPI_Info_set(simulatorinfo, "file " , filename) ;
  
if ( rank == 1 ) {printf("rank 1 process exiting...\n"); MPI_Finalize(); exit(0);};

  for ( i=0 ; i <universe; ++i ) {
    // Create a temporary application schema file
    fp = fopen(filename , "w" ) ;
    if ( fp == NULL) {
       printf(" Could not open file %s\n ", filename) ;
       MPI_Abort(MPI_COMM_WORLD,-15) ;
    }
    fprintf(fp, "c%d./simulator_errhandler_slave\n", i) ;
    fclose(fp);
    // Spawn
    MPI_Comm_spawn( 0 , MPI_ARGV_NULL, 0 , simulatorinfo, 0 , MPI_COMM_SELF, &(slave_comm[i]), &ierr);
    if (ierr != MPI_SUCCESS ) {
      printf( " Spawn Error %d\n " , ierr) ;
      MPI_Abort(MPI_COMM_WORLD,  -15);
    }
}
  MPI_Info_free(&simulatorinfo) ;
  unlink(filename) ;

  // Set error handlers on every communicator
  for ( i = 0 ; i < universe ; ++ i )
    MPI_Errhandler_set (slave_comm[i], MPI_ERRORS_RETURN ) ;
  MPI_Errhandler_set (MPI_COMM_WORLD, MPI_ERRORS_RETURN ) ;
  MPI_Errhandler_set (MPI_COMM_SELF, MPI_ERRORS_RETURN ) ;
  // Sending slave id and list of failing processes
  for ( i = 0 ; i < universe ;++ i ) {
    MPI_Send(&i , 1, MPI_INT, 0, IC_TAG, slave_comm[i]) ;
    MPI_Send(&numloop, 1 , MPI_INT , 0 , IC_TAG , slave_comm[i]) ;
    MPI_Send(&procfail, 1 , MPI_INT , 0 , IC_TAG , slave_comm[i] ) ;
    MPI_Send(failingprocs, procfail, MPI_INT, 0, IC_TAG, slave_comm[i]) ;
    //printf("sending to %d, numloop:%d, procfail:%d, failingprocs:%d\n", i, numloop, procfail, *failingprocs);
  }
  // Checking for failed processes and leaves them out while gathering data
  double sum = 0.0;
  int waitrank = 0 ;
  for ( i = 0 ; i < universe ;++ i ) {
    MPI_Irecv(&(recvval[i]), 1, MPI_DOUBLE, 0, IC_TAG ,slave_comm[i], &(recvrequest[i]));
    ierr = MPI_Recv(&(recvval[i]), 1, MPI_DOUBLE, 0, IC_TAG ,slave_comm[i], status);
    if (ierr!= MPI_SUCCESS && i >=0) {
     printf( " Not success in receiving from %d\n " , i ) ;
     recvrequest[i] = MPI_REQUEST_NULL ;
     MPI_Comm_free(&(slave_comm[i])) ;
    }else{
     printf("slave %d, recvval %f\n", i, recvval[i]);
     sum+= recvval[i];
     }
}
  for ( i = 0 ; i < universe ;++i ) {
    int waitrank = i;
    ierr = MPI_Waitany(universe , recvrequest,&waitrank, status) ;
    ierr = MPI_Waitany(universe , recvrequest,&waitrank, status) ;
    printf("waitrank is %d\n", waitrank);
    if (ierr!= MPI_SUCCESS && waitrank >=0) {
      printf( " Not success in receiving from %d\n " , waitrank ) ;
      recvrequest[waitrank] = MPI_REQUEST_NULL ;
      MPI_Comm_free(&(slave_comm[waitrank])) ;
    }else{
      printf("slave %d, recvval %f\n", waitrank, recvval[i]);
      sum+= recvval[i];
     }
  }
  printf( " The result from the sum operationis %f \n " ,sum ) ;
  fflush(stdout) ;
  MPI_Finalize( ) ;
  exit(0);
}
