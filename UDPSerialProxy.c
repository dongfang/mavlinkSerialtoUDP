// Simple UDP <--> serial proxy, there is no MAVLink interpretation and UDP datagrams
// are not aligned with MAVLink messages.

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

volatile uint8_t run = 1;

int UDPHandle;
int serialHandle;

struct sockaddr_in remoteAddr;

#define BUFSIZE 1024

uint8_t serialToUDPBuffer[BUFSIZE];
int serialToUDPBufferCnt;

uint8_t UDPToSerialBuffer[BUFSIZE];
int UDPToSerialBufferCnt;

in_addr_t remoteIP;
int remotePort;

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

static void openUDPSocket(char* remoteHostname, int _remotePort) {
	remoteAddr.sin_family = AF_INET;
	remotePort = _remotePort;
	remoteAddr.sin_port = htons(remotePort);
	remoteAddr.sin_addr.s_addr = remoteIP = inet_addr(remoteHostname);

	if (remoteAddr.sin_addr.s_addr == INADDR_NONE)
		bail("bad server address.");

	struct sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(0);

	UDPHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (UDPHandle <= 0)
		bail("failed to create socket\n");

	// We are lazy and stupid and use a single thread.
	int nonBlocking = 1;
	if (fcntl(UDPHandle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
		bail("failed to set non-blocking socket\n");
	}

	// We bind to any port.
	if (bind(UDPHandle, (struct sockaddr*) &localAddr, sizeof(struct sockaddr_in)) < 0) {
		bail("bind");
		abort();
	}
}

// return 1 if success else 0.
static uint8_t sendToUDP() {
	int sent_bytes = sendto(UDPHandle, serialToUDPBuffer, serialToUDPBufferCnt, 0, (struct sockaddr*) &remoteAddr,
			sizeof(struct sockaddr_in));

	if (sent_bytes != serialToUDPBufferCnt) {
		printf("failed to send packet: return value = %d\n", sent_bytes);
		return 0;
	}

	return 1;
}

uint8_t receiveFromUDP() {
	struct sockaddr_in from;
	uint fromLength = sizeof(from);

	UDPToSerialBufferCnt = recvfrom(UDPHandle, UDPToSerialBuffer, BUFSIZE, 0, (struct sockaddr*) &from, &fromLength);

	if (UDPToSerialBufferCnt <= 0)
		return 0;

	unsigned int from_address = from.sin_addr.s_addr; //ntohl(from.sin_addr.s_addr);
	unsigned int from_port = ntohs(from.sin_port);

	if (from_address != remoteIP || from_port != remotePort) {
		fprintf(stderr, "Strange! There was a datagram from an outside host or port.\n");
		return 0;
	} else {
		return 1;
	}
}

uint8_t sendToSerial() {
	int written = write(serialHandle, UDPToSerialBuffer, UDPToSerialBufferCnt);
	return written > 0;
}

uint8_t receiveFromSerial() {
	serialToUDPBufferCnt = read(serialHandle, serialToUDPBuffer, sizeof(serialToUDPBuffer));
	return serialToUDPBufferCnt>0;
}

void terminate() {
	run = 0;
}

uint8_t shouldReport(uint32_t total, uint32_t* tainer, int blocksize) {
	if (total==0) blocksize = 1;
	else if (total<=10) blocksize = 10;
	else if (total<=100) blocksize = 100;
		if (total >= *tainer + blocksize) {
		*tainer = total;
		return 1;
	}
	return 0;
}

void workerLoop() {
	static uint32_t totalFromSerial = 0;
	static uint32_t totalToSerial = 0;
	static uint32_t fromSerialReported = 0;
	static uint32_t toSerialReported = 0;
	while (run) {
		uint8_t got = receiveFromUDP();
		if (got) {
			totalToSerial += UDPToSerialBufferCnt;
			sendToSerial();
		}
		got = receiveFromSerial();
		if (got) {
			totalFromSerial += serialToUDPBufferCnt;
			sendToUDP();
		}

		uint8_t _print = 0;

		if (shouldReport(totalFromSerial, &fromSerialReported, 1024))
			_print = 1;

		if (shouldReport(totalToSerial, &toSerialReported, 1024))
			_print = 1;

		if (_print) {
			printf("UDP->Serial: %dk\tSerial->UDP: %dk\n", totalToSerial/1024, totalFromSerial/1024);
			_print = 0;
		}
	}
}

int main() {
	openSerialConnection("/dev/ttyUSB0", 115200);
	openUDPSocket("127.0.0.1", 14550);

	signal(SIGABRT, terminate);
	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);

	workerLoop();
}
