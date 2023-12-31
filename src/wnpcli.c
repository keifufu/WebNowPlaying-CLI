#include "wnpcli.h"
#include <stdlib.h>

static struct cag_option options[] = {
    {
        .identifier = 'p',
        .access_letters = "p",
        .access_name = "player",
        .value_name = "ID",
        .description = "The player to target. Can be active, selected, or a players ID (default: active)",
    },
    {
        .identifier = 'f',
        .access_letters = "f",
        .access_name = "format",
        .value_name = "FORMAT",
        .description = "A format string for printing properties and metadata",
    },
    {
        .identifier = 'F',
        .access_letters = "F",
        .access_name = "follow",
        .description = "Block and append the query to output when it changes",
    },
    {
        .identifier = 'l',
        .access_letters = "l",
        .access_name = "list-all",
        .description = "List the ids of all players",
    },
    {
        .identifier = 'w',
        .access_letters = "w",
        .access_name = "wait",
        .description = "Block until the event finishes",
    },
    {
        .identifier = 'h',
        .access_letters = "h",
        .access_name = "help",
        .description = "Show this help list",
    },
    {
        .identifier = 'v',
        .access_letters = "v",
        .access_name = "version",
        .description = "Print program version",
    },
};

void print_help()
{
  printf("Usage: wnpcli [OPTION...] COMMAND [ARG]\n\n");
  printf("Available Commands:\n");
  printf("  start-daemon            Starts the daemon\n");
  printf("  stop-daemon             Stops the daemon\n");
  printf("  metadata [key]          Prints metadata information\n");
  printf("  set-state [state]       Can be PLAYING, PAUSED or STOPPED\n");
  printf("  skip-previous           Skip to the previous track\n");
  printf("  skip-next               Skip to the next track\n");
  printf("  set-position [x][+/-]   Set the position or seek forward/backward x in seconds\n");
  printf("  set-volume [x][+/-]     Set the volume from 0 to 100\n");
  printf("  set-rating [x]          Set the rating from 0 to 5\n");
  printf("  set-repeat [repeat]     Set the repeat mode. Can be NONE, ALL or ONE\n");
  printf("  set-shuffle [shuffle]   Set the shuffle. Can be 0 or 1\n");
  printf("  play-pause              Toggle between playing/paused\n");
  printf("  toggle-repeat           Toggle between repeat modes\n");
  printf("  select-active           Set the selection to the active player\n");
  printf("  select-previous         Set the selection to the previous player\n");
  printf("  select-next             Set the selection to the next player\n");
  printf("\n");
  printf("Available Options:\n");
  cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
}

int player_player_id(const char* str)
{
  int start = -1;
  int i = 0;

  while (str[i] != '\0') {
    if (str[i] >= 48 && str[i] <= 57) {
      start = i;
      break;
    }
    i++;
  }

  if (start == -1) {
    return -1;
  }

  const char* numstr = str + i;
  return atoi(numstr);
}

struct arguments parse_args(int argc, char** argv)
{
  char identifier;
  cag_option_context context;
  struct arguments arguments = {PLAYER_ID_ACTIVE, "", false, false, false, -1, -1, 0};
  int param_index;
  int command_index = -1;

  cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
  while (cag_option_fetch(&context)) {
    identifier = cag_option_get(&context);
    switch (identifier) {
    case 'p': {
      const char* player_str = cag_option_get_value(&context);
      if (player_str == NULL) {
        printf("No player is was provided\n");
      }

      if (strcmp(player_str, "active") == 0) {
        arguments.player_id = PLAYER_ID_ACTIVE;
      } else if (strcmp(player_str, "selected") == 0) {
        arguments.player_id = PLAYER_ID_SELECTED;
      } else {
        int player_id = player_player_id(player_str);
        if (player_id == -1) {
          printf("Invalid player id: %s\n", player_str);
          exit(EXIT_FAILURE);
        }
        arguments.player_id = player_id;
      }

      break;
    }
    case 'f': {
      const char* format_str = cag_option_get_value(&context);
      if (format_str == NULL) {
        printf("No format string was provided\n");
        exit(EXIT_FAILURE);
      }
      strncpy(arguments.format, format_str, sizeof(arguments.format) - 1);
      break;
    }
    case 'F':
      arguments.follow = true;
      break;
    case 'l':
      arguments.list_all = true;
      break;
    case 'w':
      arguments.wait = true;
      break;
    case 'h':
      print_help();
      exit(EXIT_SUCCESS);
    case 'v':
      printf("wnpcli v%s\n", CLI_VERSION);
      exit(EXIT_SUCCESS);
      break;
    }
  }

  for (param_index = context.index; param_index < argc; ++param_index) {
    if (arguments.command == -1) {
      command_index = param_index;
      char* command = argv[param_index];
      if (strcmp(command, "start-daemon") == 0) {
        arguments.command = COMMAND_START_DAEMON;
      } else if (strcmp(command, "stop-daemon") == 0) {
        arguments.command = COMMAND_STOP_DAEMON;
      } else if (strcmp(command, "set-state") == 0) {
        arguments.command = COMMAND_SET_STATE;
      } else if (strcmp(command, "metadata") == 0) {
        arguments.command = COMMAND_METADATA;
      } else if (strcmp(command, "skip-previous") == 0) {
        arguments.command = COMMAND_SKIP_PREVIOUS;
      } else if (strcmp(command, "skip-next") == 0) {
        arguments.command = COMMAND_SKIP_NEXT;
      } else if (strcmp(command, "set-position") == 0) {
        arguments.command = COMMAND_SET_POSITION;
      } else if (strcmp(command, "set-volume") == 0) {
        arguments.command = COMMAND_SET_VOLUME;
      } else if (strcmp(command, "set-rating") == 0) {
        arguments.command = COMMAND_SET_RATING;
      } else if (strcmp(command, "set_repeat") == 0) {
        arguments.command = COMMAND_SET_REPEAT;
      } else if (strcmp(command, "set-shuffle") == 0) {
        arguments.command = COMMAND_SET_SHUFFLE;
      } else if (strcmp(command, "play-pause") == 0) {
        arguments.command = COMMAND_PLAY_PAUSE;
      } else if (strcmp(command, "toggle-repeat") == 0) {
        arguments.command = COMMAND_TOGGLE_REPEAT;
      } else if (strcmp(command, "select-active") == 0) {
        arguments.command = COMMAND_SELECT_ACTIVE;
      } else if (strcmp(command, "select-previous") == 0) {
        arguments.command = COMMAND_SELECT_PREVIOUS;
      } else if (strcmp(command, "select-next") == 0) {
        arguments.command = COMMAND_SELECT_NEXT;
      }
    } else if (arguments.command_arg == -1) {
      char* command_arg = argv[param_index];
      switch (arguments.command) {
      case COMMAND_METADATA:
        if (strcmp(command_arg, "all") == 0) {
          arguments.command_arg = METADATA_ALL;
        } else if (strcmp(command_arg, "id") == 0) {
          arguments.command_arg = METADATA_ID;
        } else if (strcmp(command_arg, "name") == 0) {
          arguments.command_arg = METADATA_NAME;
        } else if (strcmp(command_arg, "title") == 0) {
          arguments.command_arg = METADATA_TITLE;
        } else if (strcmp(command_arg, "artist") == 0) {
          arguments.command_arg = METADATA_ARTIST;
        } else if (strcmp(command_arg, "album") == 0) {
          arguments.command_arg = METADATA_ALBUM;
        } else if (strcmp(command_arg, "cover") == 0) {
          arguments.command_arg = METADATA_COVER;
        } else if (strcmp(command_arg, "cover-src") == 0) {
          arguments.command_arg = METADATA_COVER_SRC;
        } else if (strcmp(command_arg, "state") == 0) {
          arguments.command_arg = METADATA_STATE;
        } else if (strcmp(command_arg, "position") == 0) {
          arguments.command_arg = METADATA_POSITION;
        } else if (strcmp(command_arg, "position-sec") == 0) {
          arguments.command_arg = METADATA_POSITION_SEC;
        } else if (strcmp(command_arg, "duration") == 0) {
          arguments.command_arg = METADATA_DURATION;
        } else if (strcmp(command_arg, "duration-sec") == 0) {
          arguments.command_arg = METADATA_DURATION_SEC;
        } else if (strcmp(command_arg, "volume") == 0) {
          arguments.command_arg = METADATA_VOLUME;
        } else if (strcmp(command_arg, "rating") == 0) {
          arguments.command_arg = METADATA_RATING;
        } else if (strcmp(command_arg, "repeat") == 0) {
          arguments.command_arg = METADATA_REPEAT;
        } else if (strcmp(command_arg, "shuffle") == 0) {
          arguments.command_arg = METADATA_SHUFFLE;
        } else if (strcmp(command_arg, "rating-system") == 0) {
          arguments.command_arg = METADATA_RATING_SYSTEM;
        } else if (strcmp(command_arg, "available-repeat") == 0) {
          arguments.command_arg = METADATA_AVAILABLE_REPEAT;
        } else if (strcmp(command_arg, "can-set-state") == 0) {
          arguments.command_arg = METADATA_CAN_SET_STATE;
        } else if (strcmp(command_arg, "can-skip-previous") == 0) {
          arguments.command_arg = METADATA_CAN_SKIP_PREVIOUS;
        } else if (strcmp(command_arg, "can-skip-next") == 0) {
          arguments.command_arg = METADATA_CAN_SKIP_NEXT;
        } else if (strcmp(command_arg, "can-set-position") == 0) {
          arguments.command_arg = METADATA_POSITION;
        } else if (strcmp(command_arg, "can-set-volume") == 0) {
          arguments.command_arg = METADATA_CAN_SET_VOLUME;
        } else if (strcmp(command_arg, "can-set-rating") == 0) {
          arguments.command_arg = METADATA_CAN_SET_RATING;
        } else if (strcmp(command_arg, "can-set-repeat") == 0) {
          arguments.command_arg = METADATA_CAN_SET_REPEAT;
        } else if (strcmp(command_arg, "can-set-shuffle") == 0) {
          arguments.command_arg = METADATA_CAN_SET_SHUFFLE;
        } else if (strcmp(command_arg, "created-at") == 0) {
          arguments.command_arg = METADATA_CREATED_AT;
        } else if (strcmp(command_arg, "updated-at") == 0) {
          arguments.command_arg = METADATA_UPDATED_AT;
        } else if (strcmp(command_arg, "active-at") == 0) {
          arguments.command_arg = METADATA_ACTIVE_AT;
        } else if (strcmp(command_arg, "is-desktop-player") == 0) {
          arguments.command_arg = METADATA_IS_DESKTOP_PLAYER;
        } else {
          printf("Invalid metadata argument: %s\n", command_arg);
          exit(EXIT_FAILURE);
        }
        break;
      case COMMAND_SET_STATE:
        if (strcmp(command_arg, "PLAYING") == 0) {
          arguments.command_arg = WNP_STATE_PLAYING;
        } else if (strcmp(command_arg, "PAUSED") == 0) {
          arguments.command_arg = WNP_STATE_PAUSED;
        } else if (strcmp(command_arg, "STOPPED") == 0) {
          arguments.command_arg = WNP_STATE_STOPPED;
        } else {
          printf("Invalid state. Has to be PLAYING, PAUSED or STOPPED.\n");
          exit(EXIT_FAILURE);
        }
        break;
      case COMMAND_SET_POSITION: {
        arguments.command_arg = atoi(command_arg);
        char last_char = command_arg[strlen(command_arg) - 1];
        if (last_char == '+') {
          arguments.flags = arguments.flags | RELATIVE_POSITION_PLUS;
        } else if (last_char == '-') {
          arguments.flags = arguments.flags | RELATIVE_POSITION_MINUS;
        }
        break;
      }
      case COMMAND_SET_VOLUME:
        arguments.command_arg = atoi(command_arg);
        if (arguments.command_arg > 100 || arguments.command_arg < 0) {
          printf("Invalid volume: %d\n", arguments.command_arg);
          exit(EXIT_FAILURE);
        }
        char last_char = command_arg[strlen(command_arg) - 1];
        if (last_char == '+') {
          arguments.flags = arguments.flags | RELATIVE_POSITION_PLUS;
        } else if (last_char == '-') {
          arguments.flags = arguments.flags | RELATIVE_POSITION_MINUS;
        }
        break;
      case COMMAND_SET_RATING:
        arguments.command_arg = atoi(command_arg);
        if (arguments.command_arg > 5 || arguments.command_arg < 0) {
          printf("Invalid rating: %d\n", arguments.command_arg);
          exit(EXIT_FAILURE);
        }
        break;
      case COMMAND_SET_REPEAT:
        if (strcmp(command_arg, "NONE") == 0) {
          arguments.command_arg = WNP_REPEAT_NONE;
        } else if (strcmp(command_arg, "ALL") == 0) {
          arguments.command_arg = WNP_REPEAT_ALL;
        } else if (strcmp(command_arg, "ONE") == 0) {
          arguments.command_arg = WNP_REPEAT_ONE;
        } else {
          printf("Invalid repeat mode. Has to be NONE, ALL or ONE.\n");
          exit(EXIT_FAILURE);
        }
        break;
      case COMMAND_SET_SHUFFLE:
        arguments.command_arg = atoi(command_arg);
        if (arguments.command_arg != 0 && arguments.command_arg != 1) {
          printf("Invalid shuffle state: %d\n", arguments.command_arg);
          exit(EXIT_FAILURE);
        }
        break;
      }
    }
  }

  if (!arguments.list_all && arguments.command == -1) {
    printf("No command was provided.\nSee 'wnpcli --help' for more.\n");
    exit(EXIT_FAILURE);
  }

  if (command_index != -1) {
    // COMMAND_METADATA has default of -1, which is METADATA_ALL
    switch (arguments.command) {
    case COMMAND_SET_STATE:
    case COMMAND_SET_POSITION:
    case COMMAND_SET_VOLUME:
    case COMMAND_SET_RATING:
    case COMMAND_SET_REPEAT:
    case COMMAND_SET_SHUFFLE:
      if (arguments.command_arg == -1) {
        printf("No argument provided for command %s\n", argv[command_index]);
        exit(EXIT_FAILURE);
      }
      break;
    }
  }

  return arguments;
}

void no_daemon()
{
  printf("Could not connect to daemon.\nStart one with 'wnpcli start-daemon'\nRun 'wnpcli --help' to see all available commands\n");
}

bool is_daemon_running()
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    printf("WSAStartup failed\n");
    exit(EXIT_FAILURE);
  }
#endif
  int client_fd;
  struct sockaddr_un server_addr;

  client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_fd == -1) {
    return false;
  }

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, SOCKET_PATH);

  if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    return false;
  }

  close(client_fd);
#ifdef _WIN32
  WSACleanup();
#endif
  return true;
}

int connect_sock(struct arguments arguments)
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    printf("WSAStartup failed\n");
    return EXIT_FAILURE;
  }
#endif

  int client_fd;
  struct sockaddr_un server_addr;

  client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_fd == -1) {
    no_daemon();
    return EXIT_FAILURE;
  }

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, SOCKET_PATH);

  if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    no_daemon();
    return EXIT_FAILURE;
  }

  if (send(client_fd, &arguments, sizeof(struct arguments), 0) == -1) {
    no_daemon();
    return EXIT_FAILURE;
  }

  size_t message_len;
  char* message_buffer = NULL;
  while (true) {
    if (recv(client_fd, &message_len, sizeof(message_len), 0) <= 0) {
      break;
    }

    message_buffer = malloc(message_len + 1);
    if (recv(client_fd, message_buffer, message_len, 0) <= 0) {
      break;
    }

    message_buffer[message_len] = '\0';
    printf("%s\n", message_buffer);
    free(message_buffer);
    message_buffer = NULL;
  }

  if (message_buffer != NULL) {
    free(message_buffer);
  }

  // No need to close() the connection here.
  // If the while(true) loop stopped, then either
  // the connection was close from the daemon, or it
  // errored anyway.

#ifdef _WIN32
  WSACleanup();
#endif
  return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
  struct arguments arguments = parse_args(argc, argv);

  if (arguments.command == COMMAND_START_DAEMON) {
    if (is_daemon_running()) {
      printf("A daemon is already running.\nYou can stop it with 'wnpcli stop-daemon'\n");
      return EXIT_FAILURE;
    } else {
      printf("Starting the daemon\n");
      return start_daemon();
    }
  } else {
    return connect_sock(arguments);
  }
}