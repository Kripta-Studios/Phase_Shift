#include "qiskit.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#endif

#define QISKIT_SERVER_HOST "91.99.90.39"
#define QISKIT_SERVER_PORT 8609
#define QISKIT_ENDPOINT "/generate_bit"
#define QISKIT_TIMEOUT_MS 2000

static HINTERNET h_internet = NULL;
static HINTERNET h_connect = NULL;
static bool last_connected = false;

void qiskit_init(void) {
#ifdef _WIN32
  h_internet =
      InternetOpenA("PhaseShift/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
  if (h_internet) {
    DWORD timeout = QISKIT_TIMEOUT_MS;
    InternetSetOptionA(h_internet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout,
                       sizeof(timeout));
    InternetSetOptionA(h_internet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout,
                       sizeof(timeout));
    InternetSetOptionA(h_internet, INTERNET_OPTION_SEND_TIMEOUT, &timeout,
                       sizeof(timeout));

    h_connect =
        InternetConnectA(h_internet, QISKIT_SERVER_HOST, QISKIT_SERVER_PORT,
                         NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
  }
  printf("[Qiskit] Initialized connection to %s:%d\n", QISKIT_SERVER_HOST,
         QISKIT_SERVER_PORT);
#endif
}

void qiskit_shutdown(void) {
#ifdef _WIN32
  if (h_connect)
    InternetCloseHandle(h_connect);
  if (h_internet)
    InternetCloseHandle(h_internet);
  h_connect = NULL;
  h_internet = NULL;
#endif
  printf("[Qiskit] Connection closed\n");
}

int qiskit_random_bit(void) {
#ifdef _WIN32
  if (!h_connect) {
    last_connected = false;
    return rand() % 2;
  }

  HINTERNET h_request = HttpOpenRequestA(h_connect, "GET", QISKIT_ENDPOINT,
                                         NULL, NULL, NULL, 0, 0);
  if (!h_request) {
    last_connected = false;
    return rand() % 2;
  }

  BOOL sent = HttpSendRequestA(h_request, NULL, 0, NULL, 0);
  if (!sent) {
    InternetCloseHandle(h_request);
    last_connected = false;
    return rand() % 2;
  }

  char buffer[512] = {0};
  DWORD bytes_read = 0;
  DWORD total_read = 0;

  while (InternetReadFile(h_request, buffer + total_read,
                          sizeof(buffer) - total_read - 1, &bytes_read)) {
    if (bytes_read == 0)
      break;
    total_read += bytes_read;
  }
  buffer[total_read] = '\0';

  InternetCloseHandle(h_request);

  /* Parse JSON: {"success": true, "value": 0, ...} */
  /* Simple parse: find "value": and read the digit */
  char *val_ptr = strstr(buffer, "\"value\":");
  if (val_ptr) {
    val_ptr += 8; /* skip "value": */
    while (*val_ptr == ' ')
      val_ptr++;
    int bit = (*val_ptr == '1') ? 1 : 0;
    last_connected = true;
    return bit;
  }

  /* Parse failed — fallback */
  last_connected = false;
  return rand() % 2;
#else
  last_connected = false;
  return rand() % 2;
#endif
}

float qiskit_random_float(void) {
  /* Build a float from multiple quantum bits for better resolution */
  int bits = 0;
  for (int i = 0; i < 8; i++) {
    bits = (bits << 1) | qiskit_random_bit();
  }
  return (float)bits / 256.0f;
}

bool qiskit_is_connected(void) { return last_connected; }
