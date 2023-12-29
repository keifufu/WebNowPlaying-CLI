#ifndef LIBWNP_H
#define LIBWNP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WNP_MAX_PLAYERS 64
#define WNP_MAX_EVENT_RESULTS WNP_STR_LEN

enum wnp_state {
  WNP_STATE_PLAYING = 0,
  WNP_STATE_PAUSED = 1,
  WNP_STATE_STOPPED = 2,
};

enum wnp_repeat {
  WNP_REPEAT_NONE = (1 << 0),
  WNP_REPEAT_ALL = (1 << 1),
  WNP_REPEAT_ONE = (1 << 2),
};

enum wnp_rating_system {
  WNP_RATING_SYSTEM_NONE = 0,
  WNP_RATING_SYSTEM_LIKE = 1,
  WNP_RATING_SYSTEM_LIKE_DISLIKE = 2,
  WNP_RATING_SYSTEM_SCALE = 3,
};

enum wnp_event_result {
  WNP_EVENT_PENDING = 0,
  WNP_EVENT_SUCCEEDED = 1,
  WNP_EVENT_FAILED = 2,
};

/**
 * Information about a player.
 * All these fields are meant to be read-only,
 * they can update during a players lifecycle so
 * locking / unlocking the player via `wnp_lock` and
 * `wnp_unlock` is recommended when reading values.
 *
 * Actions on the player are not meant to be set here,
 * but via wnp_try... functions.
 */
#define WNP_STR_LEN 512
struct wnp_player {
  /* The players id. */
  int id;
  /**
   * The players name.
   * Eg: YouTube, Spotify...
   * If using desktop players this can be an
   * executables name (firefox.exe) or a windows appid.
   */
  char name[WNP_STR_LEN];
  /* The title. Can be empty. */
  char title[WNP_STR_LEN];
  /* The artist. Can be empty. */
  char artist[WNP_STR_LEN];
  /* The album. Can be empty. */
  char album[WNP_STR_LEN];
  /**
   * The path to the cover of the playing media.
   * Can be an empty string if no cover exists.
   * If not empty, this is always a png, starting with file://
   */
  char cover[WNP_STR_LEN];
  /**
   * The cover source of the playing media.
   * Can be:
   * - http(s)://...
   * - file://...
   * - empty
   */
  char cover_src[WNP_STR_LEN];
  /* The state of the player */
  enum wnp_state state;
  /* The position in seconds */
  int position;
  /* The duration in seconds */
  int duration;
  /* The volume from 0 to 100 */
  int volume;
  /**
   * The rating from 0 to 5.
   * Depending on the `rating_system` this
   * can mean multiple things.
   *
   * `WNP_RATING_SCALE`:
   * This is used for sites that have
   * a star-based rating system.
   *
   * `WNP_RATING_LIKE`, `WNP_RATING_LIKE_DISLIKE`:
   * 0 - unrated
   * 1 - disliked
   * 5 - liked
   */
  int rating;
  /* The repeat mode of the player */
  enum wnp_repeat repeat;
  /* Whether shuffle is enabled */
  bool shuffle;
  /**
   * The rating system.
   * More info above, in `rating`.
   */
  enum wnp_rating_system rating_system;
  /**
   * Available repeat modes.
   * This is a flag of `wnp_repeat`.
   *
   * Checking if repeat mode "ALL" is supported:
   * `player->available_repeat & WNP_REPEAT_ALL`
   */
  int available_repeat;
  /* If the state can currently be set */
  bool can_set_state;
  /* If skipping back is currently possible */
  bool can_skip_previous;
  /* If skipping next is currently possible */
  bool can_skip_next;
  /* If the position can currently be set */
  bool can_set_position;
  /* If the volume can currently be set */
  bool can_set_volume;
  /* If the rating can currently be set */
  bool can_set_rating;
  /* If the repeat mode can currently be set */
  bool can_set_repeat;
  /* If shuffle can currently be set */
  bool can_set_shuffle;
  /**
   * Timestamps in milliseconds since unix epoch (utc).
   * `created_at` - Player created
   * `updated_at` - Player updated
   * `active_at` - used in calculation for `wnp_get_active_player`
   */
  long long created_at;
  long long updated_at;
  long long active_at;
  /* If this player is a desktop player */
  bool is_desktop_player;
  /* Internal data, do not use. */
  void* _data;
};

struct wnp_events {
  void (*on_player_added)(struct wnp_player* player);
  void (*on_player_updated)(struct wnp_player* player);
  void (*on_player_removed)(struct wnp_player* player);        // player is freed right after
  void (*on_active_player_changed)(struct wnp_player* player); // player can be NULL if no active player is found
};

/**
 * Starts WebNowPlaying in a separate thread.
 *
 * port - port used for the websocket
 * adapter_version - version of the adapter in the format of x.x.x
 * events - Callback struct, can be NULL
 *
 * Returns
 *  0 - Success
 *  1 - Already started
 *  2 - Failed to start socket
 *  3 - Invalid adapter version
 *  4 - Failed to allocate memory for wnp_events
 */
int wnp_start(int port, const char* adapter_version, struct wnp_events* events);

/**
 * Stops WebNowPlaying.
 *
 * Returns
 *  0 - Success
 *  1 - Already stopped
 */
int wnp_stop();

/**
 * Whether WebNowPlaying is started or not.
 */
bool wnp_is_started();

/**
 * Locks a players mutex.
 */
void wnp_lock(struct wnp_player* player);

/**
 * Unlocks a players mutex.
 */
void wnp_unlock(struct wnp_player* player);

/**
 * Returns the player with the given id.
 * If no player was found, returns &DEFAULT_PLAYER if
 * always_return_player is true, otherwise NULL.
 */
struct wnp_player* wnp_get_player(int id, bool always_return_player);

/**
 * Tries to find an 'active' player.
 *
 * Finds either:
 * - A player that is playing, not muted, with the most recent `active_at` timestamp.
 * - The player with the most recent `active_at` timestamp.
 *
 * If no player was found, returns NULL or &DEFAULT_PLAYER,
 * depentent on `always_return_player`
 */
struct wnp_player* wnp_get_active_player(bool always_return_player);

/**
 * Gets all players and returns the amount of them.
 * Example:
 * ```
 *   struct wnp_player* players[WNP_MAX_PLAYERS];
 *   int num = wnp_get_all_players(players);
 *
 *   for (int i = 0; i < num; i++) {
 *     wnp_lock(players[i]);
 *     printf("Player name: %s\n", players[i]->name);
 *     wnp_unlock(players[i]);
 *   }
 * ```
 */
int wnp_get_all_players(struct wnp_player* players[WNP_MAX_PLAYERS]);

/**
 * Gets the result for an event.
 *
 * id - The id returned from an event function
 */
enum wnp_event_result wnp_get_event_result(int event_id);

/**
 * Block until an event result is found.
 * This checks if the event is still pending every 10ms.
 * If it is still pending after 1s it returns WNP_EVENT_FAILED.
 * The average time for an event to return a status is ~300ms.
 *
 * id - The id returned from an event function
 */
enum wnp_event_result wnp_wait_for_event_result(int event_id);

/**
 * Gets the current position in percent from 0.0f to 100.0f
 */
float wnp_get_position_percent(struct wnp_player* player);

/**
 * Gets the remaining seconds.
 */
int wnp_get_remaining_seconds(struct wnp_player* player);

/**
 * Formats seconds to a string.
 *
 * pad_with_zeroes = false:
 *  - 30 = "0:30"
 *  - 69 = "1:09"
 *  - 6969 = "1:59:09"
 *
 * pad_with_zeroes = true
 *  - 30 = "00:30"
 *  - 69 = "01:09"
 *  - 6969 = "01:56:09"
 */
void wnp_format_seconds(int seconds, bool pad_with_zeroes, char out_str[10]);

/* Base event functions */

int wnp_try_set_state(struct wnp_player* player, enum wnp_state state);
int wnp_try_skip_previous(struct wnp_player* player);
int wnp_try_skip_next(struct wnp_player* player);
int wnp_try_set_position(struct wnp_player* player, int seconds);
int wnp_try_set_volume(struct wnp_player* player, int volume);
int wnp_try_set_rating(struct wnp_player* player, int rating);
int wnp_try_set_repeat(struct wnp_player* player, enum wnp_repeat repeat);
int wnp_try_set_shuffle(struct wnp_player* player, bool shuffle);

/* Util event functions */

int wnp_try_play_pause(struct wnp_player* player);
int wnp_try_revert(struct wnp_player* player, int seconds);
int wnp_try_forward(struct wnp_player* player, int seconds);
int wnp_try_set_position_percent(struct wnp_player* player, float percent);
int wnp_try_revert_percent(struct wnp_player* player, float percent);
int wnp_try_forward_percent(struct wnp_player* player, float percent);
int wnp_try_toggle_repeat(struct wnp_player* player);

#ifdef __cplusplus
}
#endif

#endif /* LIBWNP_H */
