# MPI_ft_test

master/slave.c : original code from paper (https://brage.bibsys.no/xmlui/handle/11250/250460), which applies the idea of MPI
inter communicator error handling from here (www.mcs.anl.gov/~lusk/papers/fault-tolerance.pdf)

err_handle.c : a simple example of error handling

parent_ft/child_ft.c : MPI_spawn + error handling

Parent spawns 4 child processes and wait asynchronously for each child status.
Child process is able to catch INT signal and return or exit abnormally by logic. 
Expected: MPI ring will not die in case of child process errors, instead parent can detect ill child process and print out 
its status.


