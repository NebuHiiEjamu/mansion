#include <stdio.h>
#include <errno.h>

main(int argc, char **argv)
{
   int n;
   n = atoi(argv[1]);
   errno = n;
   perror(NULL);
   exit(n);
}
