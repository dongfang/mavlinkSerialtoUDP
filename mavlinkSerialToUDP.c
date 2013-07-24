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
#define DEFAULT_BUFFER_SIZE 256
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
	struct sockaddr_in remoteAddr;
	in_addr_t remoteIP;
	int remotePort;
} Remote;

Remote mavlinkRemote;
Remote textRemote;

size_t bufferSize = DEFAULT_BUFFER_SIZE;

uint8_t* serialToIPBuffer;
int serialToIPBufferCnt;

uint8_t* IPToSerialBuffer;
int IPToSerialBufferCnt;

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

static void openUDPSocket(struct sockaddr_in* networkAddress, int* handle) {
	if (remoteAddr->sin_addr.s_addr == INADDR_NONE)
		bail("bad server address.");

	struct sockaddr_in localAddr;

	localAddr.sin_family = AF_INET;
	// Maybe add a possibility to specify the interface to use via this.
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(0);

	*handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (*handle <= 0)
		bail("failed to create socket\n");

	// We are lazy and stupid and use a single thread.
	int nonBlocking = 1;
	if (fcntl(*handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
		bail("failed to set non-blocking socket\n");
	}

	// We bind to any port.
	if (bind(*handle, (struct sockaddr*) &localAddr, sizeof(struct sockaddr_in)) < 0) {
		bail("bind failed\n");
		abort();
	}
}

static void openSocket(char* remoteHostname, int remotePort) {
	if (mavlinkIPLink != NONE) {
		remotePort = remotePort;
		remoteAddr.sin_port = htons(remotePort);
		remoteAddr.sin_addr.s_addr = remoteIP = inet_addr(remoteHostname);
		remoteAddr.sin_family = AF_INET;

		if (mavlinkIPLink == UDP) {
			openUDPSocket();
		}

		else if (mavlinkIPLink == TCP) {
			bail("TCP not yet implemented.");
		}
	}
}

void initMAVLinkSocket(char* remoteHostname, int _remotePort) {
	openSocket
}

void allocBuffer() {
	buffer = malloc(bufferSize);
	if (buffer == NULL)
		complain;
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
