/*
 * Format:
 * foo --tty=/dev/ttyO4 --baud=19200 --remote=dongfang.ch:14550 --remote=udp:localhost:14550 --remote=tcp:192.168.1.1:10000 --mavlink --logging
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>

typedef uint8_t bool;
#define true 1
#define false 0

int remoteCount;

void parsePort(char* arg) {
  printf("Port is : %s\n", arg);
}

void parseRemote(char* arg) {
  printf("Remote is : %s\n", arg);
}

int main(int argc, char** argv) {
  bool isMAVLink = false;
  bool doesLogging = false;
  bool didGetPort = false;
  bool didGetRemote = false;
  int c;

  struct option longopts[2] = {
    (struct option){.name="port", .has_arg=1, .flag=NULL, .val=200},
    (struct option){.name=NULL, .has_arg=0, .flag=NULL, .val=0}
  };

  while ((c = getopt_long(argc, argv, "p:r:md:", longopts, NULL)) != -1) {
    switch (c)
      {
      case 'p':
      case 200:
	parsePort(optarg);
	didGetPort = true;
	break;
      case 'r':
	parseRemote(optarg);
	didGetRemote = true;
	break;
      case 'm':
	isMAVLink = true;
	break;
      case 'd':
	doesLogging = true;
	break;
	/*
if (optopt == 'c')
	  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	else if (isprint (optopt))
	  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	else
	  fprintf (stderr,
		   "Unknown option character `\\x%x'.\n",
		   optopt);
	return 1;
	*/
      default:
	abort ();
      }
  }
}
