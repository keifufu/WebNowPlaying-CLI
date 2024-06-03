#include "wnp.h"
#include "wnpcli.h"

/**
 * How would "seleted" player work
 *
 * All instances of wnpcli clients share the same selection as the daemon will keep track of it.
 * This is fine because its one user anyway who would want the selection the same across all instances.
 *
 * selected player starts as active player
 * "select-prev" selects closest previous index
 * "select-next" selects clostest next index
 * "select-active" selects active player or default_player
 *
 * if selected player is removed, fallback to active player (which will fallback to DEFAULT_PLAYER)
 *
 */

#define MAX_RESPONSE_LEN 2048
struct client_state {
  struct arguments arguments;
  char response[MAX_RESPONSE_LEN];
  bool should_close;
  int client_fd;
  long long player_last_updated_at;
};

#define MAX_STATES 64
struct client_state* g_states[MAX_STATES] = {0};
int g_selected_player_id = PLAYER_ID_ACTIVE;

void send_message(int client_fd, const char* message)
{
  if (message != NULL) {
    size_t message_len = strlen(message);
    send(client_fd, &message_len, sizeof(message_len), 0);
    send(client_fd, message, message_len, 0);
  }
}

struct wnp_player* get_player_from_state(struct client_state* state)
{
  switch (state->arguments.player_id) {
  case PLAYER_ID_ACTIVE:
    return wnp_get_active_player(true);
  case PLAYER_ID_SELECTED:
    if (g_selected_player_id == PLAYER_ID_ACTIVE) {
      return wnp_get_active_player(true);
    } else {
      struct wnp_player* player = wnp_get_player(g_selected_player_id, false);
      if (player == NULL) {
        g_selected_player_id = PLAYER_ID_ACTIVE;
        return wnp_get_active_player(true);
      } else {
        return player;
      }
    }
    break;
  default:
    if (state->arguments.player_id >= WNP_MAX_PLAYERS) {
      return wnp_get_player(0, true);
    } else {
      return wnp_get_player(state->arguments.player_id, true);
    }
  }
}

/**
 * On windows, taskkill and taskmgr don't fire
 * SIGTERM, so uh, cope? If you taskkill it then
 * you better know what you're doing anyway.
 *
 * It sends WM_CLOSE but we have no windows so meh.
 **/
void signal_handler(int signum)
{
  wnp_stop();
  exit(0);
}

void replace_placeholder(char* str, const char* placeholder, char* value)
{
  char* found = strstr(str, placeholder);
  if (found != NULL) {
    memmove(found + strlen(value), found + strlen(placeholder), strlen(found + strlen(placeholder)) + 1);
    memcpy(found, value, strlen(value));
  }
}

void get_formatted_id(struct wnp_player* player, char* output, size_t size)
{
  char* name_lowercase = strdup(player->name);
  char* dst = name_lowercase;
  for (char* src = name_lowercase; *src; ++src) {
    if (*src != ' ') {
      *dst = tolower(*src);
      ++dst;
    }
  }
  *dst = '\0';
  snprintf(output, size, "%s%d", name_lowercase, player->id);
  free(name_lowercase);
}

void append_response(struct client_state* state, char* key, char* value)
{
  char formatted[MAX_RESPONSE_LEN];
  snprintf(formatted, MAX_RESPONSE_LEN, "%-30s %s\n", key, value);
  if (strlen(state->response) + strlen(formatted) < MAX_RESPONSE_LEN) {
    strncat(state->response, formatted, MAX_RESPONSE_LEN - strlen(state->response) - 1);
  }
}

// Very naive implementation, but it works for now soooo.... can I be bothered?
void compute_metadata(struct client_state* state, struct wnp_player* player)
{
  char id_str[MAX_RESPONSE_LEN] = "";
  char name_str[MAX_RESPONSE_LEN] = "";
  char title_str[MAX_RESPONSE_LEN] = "";
  char artist_str[MAX_RESPONSE_LEN] = "";
  char album_str[MAX_RESPONSE_LEN] = "";
  char cover_str[MAX_RESPONSE_LEN] = "";
  char cover_src_str[MAX_RESPONSE_LEN] = "";
  char state_str[MAX_RESPONSE_LEN] = "";
  char position_str[MAX_RESPONSE_LEN] = "";
  char position_sec_str[MAX_RESPONSE_LEN] = "";
  char duration_str[MAX_RESPONSE_LEN] = "";
  char duration_sec_str[MAX_RESPONSE_LEN] = "";
  char volume_str[MAX_RESPONSE_LEN] = "";
  char rating_str[MAX_RESPONSE_LEN] = "";
  char repeat_str[MAX_RESPONSE_LEN] = "";
  char shuffle_str[MAX_RESPONSE_LEN] = "";
  char rating_system_str[MAX_RESPONSE_LEN] = "";
  char available_repeat_str[MAX_RESPONSE_LEN] = "";
  char can_set_state_str[MAX_RESPONSE_LEN] = "";
  char can_skip_previous_str[MAX_RESPONSE_LEN] = "";
  char can_skip_next_str[MAX_RESPONSE_LEN] = "";
  char can_set_position_str[MAX_RESPONSE_LEN] = "";
  char can_set_volume_str[MAX_RESPONSE_LEN] = "";
  char can_set_rating_str[MAX_RESPONSE_LEN] = "";
  char can_set_repeat_str[MAX_RESPONSE_LEN] = "";
  char can_set_shuffle_str[MAX_RESPONSE_LEN] = "";
  char created_at_str[MAX_RESPONSE_LEN] = "";
  char updated_at_str[MAX_RESPONSE_LEN] = "";
  char active_at_str[MAX_RESPONSE_LEN] = "";
  char is_desktop_player_str[MAX_RESPONSE_LEN] = "";

  wnp_lock(player);
  char formatted_id[MAX_RESPONSE_LEN] = "";
  get_formatted_id(player, formatted_id, sizeof(formatted_id));
  snprintf(id_str, MAX_RESPONSE_LEN, "%s", formatted_id);
  snprintf(name_str, MAX_RESPONSE_LEN, "%s", player->name);
  snprintf(title_str, MAX_RESPONSE_LEN, "%s", player->title);
  snprintf(artist_str, MAX_RESPONSE_LEN, "%s", player->artist);
  snprintf(album_str, MAX_RESPONSE_LEN, "%s", player->album);
  snprintf(cover_str, MAX_RESPONSE_LEN, "%s", player->cover);
  snprintf(cover_src_str, MAX_RESPONSE_LEN, "%s", player->cover_src);
  char* state_values[] = {"playing", "paused", "stopped"};
  snprintf(state_str, MAX_RESPONSE_LEN, "%s", state_values[player->state]);
  wnp_format_seconds(player->position, false, position_str);
  snprintf(position_sec_str, MAX_RESPONSE_LEN, "%d", player->position);
  wnp_format_seconds(player->duration, false, duration_str);
  snprintf(duration_sec_str, MAX_RESPONSE_LEN, "%d", player->duration);
  snprintf(volume_str, MAX_RESPONSE_LEN, "%d", player->volume);
  snprintf(rating_str, MAX_RESPONSE_LEN, "%d", player->rating);
  char* repeat_str_values[] = {"", "none", "all", "", "one"};
  snprintf(repeat_str, MAX_RESPONSE_LEN, "%s", repeat_str_values[player->repeat]);
  snprintf(shuffle_str, MAX_RESPONSE_LEN, "%s", player->shuffle ? "true" : "false");
  char* rating_systems_values[] = {"none", "like", "like-dislike", "scale"};
  snprintf(rating_system_str, MAX_RESPONSE_LEN, "%s", rating_systems_values[player->rating_system]);
  snprintf(available_repeat_str, MAX_RESPONSE_LEN, "%d", player->available_repeat);
  snprintf(can_set_state_str, MAX_RESPONSE_LEN, "%s", player->can_set_state ? "true" : "false");
  snprintf(can_skip_previous_str, MAX_RESPONSE_LEN, "%s", player->can_skip_previous ? "true" : "false");
  snprintf(can_skip_next_str, MAX_RESPONSE_LEN, "%s", player->can_skip_next ? "true" : "false");
  snprintf(can_set_position_str, MAX_RESPONSE_LEN, "%s", player->can_set_position ? "true" : "false");
  snprintf(can_set_volume_str, MAX_RESPONSE_LEN, "%s", player->can_set_volume ? "true" : "false");
  snprintf(can_set_rating_str, MAX_RESPONSE_LEN, "%s", player->can_set_rating ? "true" : "false");
  snprintf(can_set_repeat_str, MAX_RESPONSE_LEN, "%s", player->can_set_repeat ? "true" : "false");
  snprintf(can_set_shuffle_str, MAX_RESPONSE_LEN, "%s", player->can_set_shuffle ? "true" : "false");
  snprintf(created_at_str, MAX_RESPONSE_LEN, "%lld", player->created_at);
  snprintf(updated_at_str, MAX_RESPONSE_LEN, "%lld", player->updated_at);
  snprintf(active_at_str, MAX_RESPONSE_LEN, "%lld", player->active_at);
  snprintf(is_desktop_player_str, MAX_RESPONSE_LEN, "%s", player->is_desktop_player ? "true" : "false");
  wnp_unlock(player);

  if (strlen(state->arguments.format) > 0) {
    strncpy(state->response, state->arguments.format, MAX_RESPONSE_LEN);
    replace_placeholder(state->response, "{{id}}", id_str);
    replace_placeholder(state->response, "{{name}}", name_str);
    replace_placeholder(state->response, "{{title}}", title_str);
    replace_placeholder(state->response, "{{artist}}", artist_str);
    replace_placeholder(state->response, "{{album}}", album_str);
    replace_placeholder(state->response, "{{cover}}", cover_str);
    replace_placeholder(state->response, "{{cover-src}}", cover_src_str);
    replace_placeholder(state->response, "{{state}}", state_str);
    replace_placeholder(state->response, "{{position}}", position_str);
    replace_placeholder(state->response, "{{position-sec}}", position_sec_str);
    replace_placeholder(state->response, "{{duration}}", duration_str);
    replace_placeholder(state->response, "{{duration-sec}}", duration_sec_str);
    replace_placeholder(state->response, "{{volume}}", volume_str);
    replace_placeholder(state->response, "{{rating}}", rating_str);
    replace_placeholder(state->response, "{{repeat}}", repeat_str);
    replace_placeholder(state->response, "{{shuffle}}", shuffle_str);
    replace_placeholder(state->response, "{{rating-system}}", rating_system_str);
    replace_placeholder(state->response, "{{available-repeat}}", available_repeat_str);
    replace_placeholder(state->response, "{{can-set-state}}", can_set_state_str);
    replace_placeholder(state->response, "{{can-skip-previous}}", can_skip_previous_str);
    replace_placeholder(state->response, "{{can-skip-next}}", can_skip_next_str);
    replace_placeholder(state->response, "{{can-set-position}}", can_set_position_str);
    replace_placeholder(state->response, "{{can-set-volume}}", can_set_volume_str);
    replace_placeholder(state->response, "{{can-set-rating}}", can_set_rating_str);
    replace_placeholder(state->response, "{{can-set-repeat}}", can_set_repeat_str);
    replace_placeholder(state->response, "{{can-set-shuffle}}", can_set_shuffle_str);
    replace_placeholder(state->response, "{{created-at}}", created_at_str);
    replace_placeholder(state->response, "{{updated-at}}", updated_at_str);
    replace_placeholder(state->response, "{{active-at}}", active_at_str);
    replace_placeholder(state->response, "{{is-desktop-player}}", is_desktop_player_str);
    return;
  }

  switch (state->arguments.command_arg) {
  case METADATA_ALL:
    append_response(state, "id", id_str);
    append_response(state, "name", name_str);
    append_response(state, "title", title_str);
    append_response(state, "artist", artist_str);
    append_response(state, "album", album_str);
    append_response(state, "cover", cover_str);
    append_response(state, "cover-src", cover_src_str);
    append_response(state, "state", state_str);
    append_response(state, "position", position_str);
    append_response(state, "position-sec", position_sec_str);
    append_response(state, "duration", duration_str);
    append_response(state, "duration-sec", duration_sec_str);
    append_response(state, "volume", volume_str);
    append_response(state, "rating", rating_str);
    append_response(state, "repeat", repeat_str);
    append_response(state, "shuffle", shuffle_str);
    append_response(state, "rating-system", rating_system_str);
    append_response(state, "available-repeat", available_repeat_str);
    append_response(state, "can-set-state", can_set_state_str);
    append_response(state, "can-skip-previous", can_skip_previous_str);
    append_response(state, "can-skip-next", can_skip_next_str);
    append_response(state, "can-set-position", can_set_position_str);
    append_response(state, "can-set-volume", can_set_volume_str);
    append_response(state, "can-set-rating", can_set_rating_str);
    append_response(state, "can-set-repeat", can_set_repeat_str);
    append_response(state, "can-set-shuffle", can_set_shuffle_str);
    append_response(state, "created-at", created_at_str);
    append_response(state, "updated-at", updated_at_str);
    append_response(state, "active-at", active_at_str);
    append_response(state, "is-desktop-player", is_desktop_player_str);
    break;
  case METADATA_ID:
    strncpy(state->response, id_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_NAME:
    strncpy(state->response, name_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_TITLE:
    strncpy(state->response, title_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_ARTIST:
    strncpy(state->response, artist_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_ALBUM:
    strncpy(state->response, album_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_COVER:
    strncpy(state->response, cover_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_COVER_SRC:
    strncpy(state->response, cover_src_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_STATE:
    strncpy(state->response, state_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_POSITION:
    strncpy(state->response, position_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_POSITION_SEC:
    strncpy(state->response, position_sec_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_DURATION:
    strncpy(state->response, duration_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_DURATION_SEC:
    strncpy(state->response, duration_sec_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_VOLUME:
    strncpy(state->response, volume_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_RATING:
    strncpy(state->response, rating_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_REPEAT:
    strncpy(state->response, repeat_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_SHUFFLE:
    strncpy(state->response, shuffle_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_RATING_SYSTEM:
    strncpy(state->response, rating_system_str, MAX_RESPONSE_LEN);
  case METADATA_AVAILABLE_REPEAT:
    strncpy(state->response, available_repeat_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SET_STATE:
    strncpy(state->response, can_set_state_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SKIP_PREVIOUS:
    strncpy(state->response, can_skip_previous_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SKIP_NEXT:
    strncpy(state->response, can_skip_next_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SET_POSITION:
    strncpy(state->response, can_set_position_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SET_VOLUME:
    strncpy(state->response, can_set_volume_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SET_RATING:
    strncpy(state->response, can_set_rating_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SET_REPEAT:
    strncpy(state->response, can_set_repeat_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CAN_SET_SHUFFLE:
    strncpy(state->response, can_set_shuffle_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_CREATED_AT:
    strncpy(state->response, created_at_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_UPDATED_AT:
    strncpy(state->response, updated_at_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_ACTIVE_AT:
    strncpy(state->response, active_at_str, MAX_RESPONSE_LEN);
    break;
  case METADATA_IS_DESKTOP_PLAYER:
    strncpy(state->response, is_desktop_player_str, MAX_RESPONSE_LEN);
    break;
  }
}

void compute_state(struct client_state* state)
{
  if (state->arguments.list_all) {
    struct wnp_player* players[WNP_MAX_PLAYERS];
    int num = wnp_get_all_players(players);
    char player_info[MAX_RESPONSE_LEN];
    memset(player_info, 0, sizeof(player_info));

    for (int i = 0; i < num; i++) {
      if (players[i] != NULL) {
        char info[MAX_RESPONSE_LEN];
        wnp_lock(players[i]);
        char formatted_id[MAX_RESPONSE_LEN] = "";
        get_formatted_id(players[i], formatted_id, sizeof(formatted_id));
        snprintf(info, MAX_RESPONSE_LEN, "%s %s\n", formatted_id, players[i]->name);
        wnp_unlock(players[i]);
        strncat(player_info, info, MAX_RESPONSE_LEN - strlen(player_info) - 1);
      }
    }

    strncpy(state->response, player_info, MAX_RESPONSE_LEN);
    state->should_close = true;
    return;
  }

  struct wnp_player* player = get_player_from_state(state);
  int event_id = -1;

  switch (state->arguments.command) {
  case COMMAND_STOP_DAEMON:
    // Need to send the messages in here instead of after compute_state
    // since we stop the daemon here.
    printf("Received stop-daemon. Stopping...\n");
    send_message(state->client_fd, "Stopping the daemon...\nDone.");
    signal_handler(SIGTERM);
    break;
  case COMMAND_METADATA:
    compute_metadata(state, player);
    state->should_close = !state->arguments.follow;
    break;
  case COMMAND_SET_STATE:
    event_id = wnp_try_set_state(player, state->arguments.command);
    break;
  case COMMAND_SKIP_PREVIOUS:
    event_id = wnp_try_skip_previous(player);
    break;
  case COMMAND_SKIP_NEXT:
    event_id = wnp_try_skip_next(player);
    break;
  case COMMAND_SET_POSITION:
    if (state->arguments.flags & RELATIVE_POSITION_PLUS) {
      event_id = wnp_try_forward(player, state->arguments.command_arg);
    } else if (state->arguments.flags & RELATIVE_POSITION_MINUS) {
      event_id = wnp_try_revert(player, state->arguments.command_arg);
    } else {
      event_id = wnp_try_set_position(player, state->arguments.command_arg);
    }
    break;
  case COMMAND_SET_VOLUME:
    if (state->arguments.flags & RELATIVE_POSITION_PLUS) {
      event_id = wnp_try_set_volume(player, state->arguments.command_arg + player->volume);
    } else if (state->arguments.flags & RELATIVE_POSITION_MINUS) {
      event_id = wnp_try_set_volume(player, state->arguments.command_arg - player->volume);
    } else {
      event_id = wnp_try_set_volume(player, state->arguments.command_arg);
    }
    break;
  case COMMAND_SET_RATING:
    event_id = wnp_try_set_rating(player, state->arguments.command_arg);
    break;
  case COMMAND_SET_REPEAT:
    event_id = wnp_try_set_repeat(player, state->arguments.command_arg);
    break;
  case COMMAND_SET_SHUFFLE:
    event_id = wnp_try_set_shuffle(player, state->arguments.command_arg);
    break;
  case COMMAND_PLAY_PAUSE:
    event_id = wnp_try_play_pause(player);
    break;
  case COMMAND_TOGGLE_REPEAT:
    event_id = wnp_try_toggle_repeat(player);
    break;
  case COMMAND_SELECT_ACTIVE:
    g_selected_player_id = PLAYER_ID_ACTIVE;
    break;
  case COMMAND_SELECT_PREVIOUS: {
    bool found = false;

    // Search between <current> and 0
    for (int i = player->id; i > 0; i--) {
      struct wnp_player* new_player = wnp_get_player(i, false);
      if (new_player != NULL) {
        g_selected_player_id = new_player->id;
        found = true;
        break;
      }
    }

    if (!found) {
      // Search between <max> and <current>
      for (int i = WNP_MAX_PLAYERS; i > player->id; i--) {
        struct wnp_player* new_player = wnp_get_player(i, false);
        if (new_player != NULL) {
          g_selected_player_id = new_player->id;
          found = true;
          break;
        }
      }
    }

    state->should_close = true;
    if (!found) {
      snprintf(state->response, MAX_RESPONSE_LEN, "No player to select was found");
    } else {
      char formatted_id[MAX_RESPONSE_LEN] = "";
      wnp_lock(player);
      get_formatted_id(player, formatted_id, sizeof(formatted_id));
      wnp_unlock(player);
      snprintf(state->response, MAX_RESPONSE_LEN, "Selected player %s", formatted_id);
    }
    break;
  }
  case COMMAND_SELECT_NEXT: {
    bool found = false;

    // Search between <current> and <max>
    for (int i = player->id; i < WNP_MAX_PLAYERS; i++) {
      struct wnp_player* new_player = wnp_get_player(i, false);
      if (new_player != NULL) {
        g_selected_player_id = new_player->id;
        found = true;
        break;
      }
    }

    if (!found) {
      // Search between 0 and <current>
      for (int i = 0; i < player->id; i++) {
        struct wnp_player* new_player = wnp_get_player(i, false);
        if (new_player != NULL) {
          g_selected_player_id = new_player->id;
          found = true;
          break;
        }
      }
    }

    state->should_close = true;
    if (!found) {
      snprintf(state->response, MAX_RESPONSE_LEN, "No player to select was found");
    } else {
      char formatted_id[MAX_RESPONSE_LEN] = "";
      wnp_lock(player);
      get_formatted_id(player, formatted_id, sizeof(formatted_id));
      wnp_unlock(player);
      snprintf(state->response, MAX_RESPONSE_LEN, "Selected player %s", formatted_id);
    }
    break;
  }
  }

  if (event_id != -1) {
    if (state->arguments.wait) {
      enum wnp_event_result result = wnp_wait_for_event_result(event_id);
      char* responses[] = {"PENDING", "SUCCEEDED", "FAILED"};
      snprintf(state->response, MAX_RESPONSE_LEN, "%s", responses[result]);
      state->should_close = true;
      return;
    } else {
      snprintf(state->response, MAX_RESPONSE_LEN, "%d", event_id);
      state->should_close = true;
      return;
    }
  }
}

void on_any_wnp_update(struct wnp_player* player)
{
  for (int i = 0; i < MAX_STATES; i++) {
    struct client_state* state = g_states[i];
    if (state != NULL) {
      struct wnp_player* player = get_player_from_state(state);
      if (state->player_last_updated_at != player->updated_at) {
        char* last_response = strdup(state->response);
        compute_state(state);
        if (strcmp(last_response, state->response) != 0) {
          send_message(state->client_fd, state->response);
        }
        state->player_last_updated_at = player->updated_at;
        free(last_response);
      }
    }
  }
}

int handle_client(void* data)
{
  int client_fd = *((int*)data);
  struct client_state state = {{-1, "", false, false, false, -1, -1}, "", false, client_fd, 0};

  if (recv(client_fd, &state.arguments, sizeof(struct arguments), 0) <= 0) {
    return 0;
  } else {
    compute_state(&state);
    send_message(client_fd, state.response);

    if (state.should_close) {
      close(client_fd);
      return 0;
    }

    int index = -1;
    for (int i = 0; i < MAX_STATES; i++) {
      if (g_states[i] == NULL) {
        index = i;
        g_states[i] = &state;
        break;
      }
    }

    if (index == -1) {
      send_message(client_fd, "Too many clients connected");
      close(client_fd);
      return 0;
    }

    recv(client_fd, NULL, 0, 0);
    g_states[index] = NULL;
  }

  return 0;
}

int start_daemon()
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    perror("WSAStartup failed");
    return -1;
  }
#endif

  for (int i = 0; i < MAX_STATES; i++) {
    g_states[i] = NULL;
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  struct wnp_events events;
  events.on_player_added = &on_any_wnp_update;
  events.on_player_updated = &on_any_wnp_update;
  events.on_player_removed = &on_any_wnp_update;
  events.on_active_player_changed = &on_any_wnp_update;
  int wnp_ret = wnp_start(CLI_PORT, CLI_VERSION, &events);
  if (wnp_ret > 0) {
    fprintf(stderr, "Failed to start webnowplaying with code %d\n", wnp_ret);
    exit(-1);
  }

  int server_fd, client_fd;
  struct sockaddr_un server_addr, client_addr;
  thread_ptr_t thread;

  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("Failed to create a socket");
    return -1;
  }

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, get_socket_path());
  unlink(get_socket_path());

  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Failed to bind the socket");
    return -1;
  }

  listen(server_fd, 5);

  while (1) {
    int len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
    if (client_fd == -1) {
      perror("Failed to accept client");
      return -1;
    }

    thread = thread_create(handle_client, &client_fd, THREAD_STACK_SIZE_DEFAULT);
    thread_detach(thread);
  }

  close(server_fd);
#ifdef _WIN32
  WSACleanup();
#endif
  signal_handler(SIGTERM);
}