#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

// There should be on reason to have a larger buffer
// than what is enough for one MAVLink message.
#define DEFAULT_BUFFER_SIZE 1024
#define DEFAULT_PORT 14550
#define DEFAULT_BAUD 57600

typedef uint8_t bool;

typedef enum {
	NONE = 0, UDP, TCP
} LinkType;

bool sendTextMessages;

/* 
 * True if MAVLink messages should be attempted interpreted (to align 
 * complete MAVLink messages to UDP messages and possibly also do text
 * logging of them)
 */
bool interpretMAVLink;

typedef struct {
	LinkType linkType;
	in_addr_t remoteIP;
	struct sockaddr_in remoteAddr;
	struct sockaddr_in localAddr;
	int remotePort;
	int handle;

	uint8_t inBuffer[DEFAULT_BUFFER_SIZE];
	uint8_t outBuffer[DEFAULT_BUFFER_SIZE];

} Remote;

Remote mavlinkRemote[4];
int numberOfRemotes;

// Text messages. Is pretty pointless if MAVProxy is run on-board, as MAVProxy will do the same.
Remote textRemote;

static void bail(const char *on_what) {
	fputs(strerror(errno), stderr);
	fputs(": ", stderr);
	fputs(on_what, stderr);
	fputc('\n', stderr);
	exit(1);
}

static void openSerialConnection(char* name, speed_t baudRate) {
	serialHandle = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
	fcntl(serialHandle, F_SETFL, FNDELAY);

	struct termios options;
	/* get the current options */
	tcgetattr(serialHandle, &options);

	/* set raw input, 1 second timeout */
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;

	cfsetspeed(&options, baudRate);

	/* set the options */
	tcsetattr(serialHandle, TCSANOW, &options);
}

static void openUDPSocket(Remote* remote) {
	if (remote->remoteIP == INADDR_NONE)
		bail("bad server address.");

	remote->localAddr.sin_family = AF_INET;

	// Maybe add a possibility to specify the interface to use via this. We just pick
	// whichever one now.
	remote->localAddr.sin_addr = INADDR_ANY;
	remote->localAddr.sin_port = htons(0);

	remote->handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (remote->handle <= 0)
		bail("failed to create socket\n");

	// We are lazy and stupid and use a single thread.
	int nonBlocking = 1;
	if (fcntl(remote->handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
		bail("failed to set non-blocking socket\n");
	}

	// We bind to any port.
	if (bind(remote->handle, (struct sockaddr*) remote->localAddr, sizeof(struct sockaddr_in)) < 0) {
		bail("bind failed\n");
		abort();
	}
}

static void openTCPSocket(Remote* remote) {

}

static void openSocket(Remote* remote) {
	if (remote->linkType == UDP)
		openUDPSocket(remote);
	else if (remote->linkType == TCP)
		openTCPSocket(remote);
}

void receiveByte(uint8_t data) {
}

bool shouldSendImmediately() {
	// buffer full or complete message or timeout.
	// Or better: buffer full or (complete message and buffer not ridiculously low) or timeout
	return true;
}

void mavlinkThread() {

}
