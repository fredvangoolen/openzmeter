#pragma once
#include <stdbool.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


#define REQUESTSERVER_PATH       "/var/run/openZmeter.sock"
#define REQUESTSERVER_BUFSIZE     128
#define REQUESTSERVER_PERM       0666
#define REQUESTSERVER_CLIENTS      50





