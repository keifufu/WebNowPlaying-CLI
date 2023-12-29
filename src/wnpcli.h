#include "cargs.h"
#include "thread.h"
#include "wnp.h"
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: actual cli port is 5468 but its in use right now
#define CLI_PORT 1234
#define CLI_VERSION "3.0.0"

#ifdef _WIN32
#define SOCKET_PATH "wnpcli.sock"

#include <windows.h>

#include <afunix.h>
#include <winsock2.h>

#define unlink _unlink
#else
#define SOCKET_PATH "/tmp/wnpcli.sock"

#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#endif

enum commands {
  COMMAND_START_DAEMON,
  COMMAND_STOP_DAEMON,
  COMMAND_METADATA,
  COMMAND_SET_STATE,
  COMMAND_SKIP_PREVIOUS,
  COMMAND_SKIP_NEXT,
  COMMAND_SET_POSITION,
  COMMAND_SET_VOLUME,
  COMMAND_SET_RATING,
  COMMAND_SET_REPEAT,
  COMMAND_SET_SHUFFLE,
  COMMAND_PLAY_PAUSE,
  COMMAND_TOGGLE_REPEAT,
  COMMAND_SELECT_ACTIVE,
  COMMAND_SELECT_PREVIOUS,
  COMMAND_SELECT_NEXT,
};

enum player_id {
  PLAYER_ID_ACTIVE = -1,
  PLAYER_ID_SELECTED = -2,
};

enum metadata {
  METADATA_ALL = -1,
  METADATA_ID,
  METADATA_NAME,
  METADATA_TITLE,
  METADATA_ARTIST,
  METADATA_ALBUM,
  METADATA_COVER,
  METADATA_COVER_SRC,
  METADATA_STATE,
  METADATA_POSITION,
  METADATA_POSITION_SEC,
  METADATA_DURATION,
  METADATA_DURATION_SEC,
  METADATA_VOLUME,
  METADATA_RATING,
  METADATA_REPEAT,
  METADATA_SHUFFLE,
  METADATA_RATING_SYSTEM,
  METADATA_AVAILABLE_REPEAT,
  METADATA_CAN_SET_STATE,
  METADATA_CAN_SKIP_PREVIOUS,
  METADATA_CAN_SKIP_NEXT,
  METADATA_CAN_SET_POSITION,
  METADATA_CAN_SET_VOLUME,
  METADATA_CAN_SET_RATING,
  METADATA_CAN_SET_REPEAT,
  METADATA_CAN_SET_SHUFFLE,
  METADATA_CREATED_AT,
  METADATA_UPDATED_AT,
  METADATA_ACTIVE_AT,
  METADATA_IS_DESKTOP_PLAYER,
};

enum cli_flags {
  RELATIVE_POSITION_PLUS = (1 << 0),
  RELATIVE_POSITION_MINUS = (1 << 1),
};

struct arguments {
  int player_id;
  char format[256];
  bool follow;
  bool list_all;
  bool wait;
  int command;
  int command_arg;
  int flags;
};

extern int start_daemon();
