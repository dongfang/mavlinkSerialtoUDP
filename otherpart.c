/*
 * Format:
 * foo --tty=/dev/ttyO4 --baud=19200 --remote=dongfang.ch:14550 --remote=udp:localhost:14550 --remote=tcp:192.168.1.1:10000 --mavlink --logging
 */

//#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>

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
  char port[100];
  int baudrate = 57600;

  int c;

  struct option longopts[] = {
    (struct option){.name="port", .has_arg=1, .flag=NULL, .val=200},
    (struct option){.name="remote", .has_arg=1, .flag=NULL, .val=201},
    (struct option){.name="baudrate", .has_arg=1, .flag=NULL, .val=202},
    (struct option){.name="mavlink", .has_arg=0, .flag=NULL, .val=203},
    (struct option){.name="debug", .has_arg=1, .flag=NULL, .val=204},
    (struct option){.name=NULL, .has_arg=0, .flag=NULL, .val=0}
  };

  while ((c = getopt_long(argc, argv, "p:r:mdb:", longopts, NULL)) != -1) {
    switch (c)
      {
      case 'p':
      case 200:
	strcpy(port, optarg);
	didGetPort = true;
	break;
      case 'b':
      case 202:
	baudrate = atoi(optarg);
      case 'r':
      case 201:
	parseRemote(optarg);
	didGetRemote = true;
	break;
      case 'm':
      case 203:
	isMAVLink = true;
	break;
      case 'd':
      case 204:
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
