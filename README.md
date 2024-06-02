# WebNowPlaying CLI

A CLI adapter for [WebNowPlaying](https://github.com/keifufu/WebNowPlaying).  
Support for desktop players is limited to Windows as of now.

# Installing

### Manually

Download from [Releases](https://github.com/keifufu/WebNowPlaying-CLI/releases)

### Nix

```bash
nix profile install github:keifufu/WebNowPlaying-CLI
```

<details>
<summary>NixOS</summary>

This assumes you use home manager with flakes.

- Add `github:keifufu/WebNowPlaying-CLI` to your inputs as `wnpcli`
- Import `inputs.wnpcli.homeManagerModules.wnpcli`

```nix
  programs.wnpcli = {
    enable = true;
    # If you want to start the wnpcli daemon on startup
    service.enable = true;
  };
```

</details>

# Usage

Currently, the daemon has to be started manually, with `wnpcli start-daemon`.  
This might change in the future if I package the software / create an installer.

```console
Usage: wnpcli [OPTION...] COMMAND [ARG]

Available Commands:
  start-daemon            Starts the daemon
  stop-daemon             Stops the daemon
  metadata [key]          Prints metadata information
  set-state [state]       Can be PLAYING, PAUSED or STOPPED
  skip-previous           Skip to the previous track
  skip-next               Skip to the next track
  set-position [x][+/-]   Set the position or seek forward/backward x in seconds
  set-volume [x]          Set the volume from 0 to 100
  set-rating [x]          Set the rating from 0 to 5
  set-repeat [repeat]     Set the repeat mode. Can be NONE, ALL or ONE
  set-shuffle [shuffle]   Set the shuffle. Can be 0 or 1
  play-pause              Toggle between playing/paused
  toggle-repeat           Toggle between repeat modes
  select-active           Set the selection to the active player
  select-previous         Set the selection to the previous player
  select-next             Set the selection to the next player

Available Options:
  -p, --player=ID         The player to target. Can be active, selected, or a players ID (default: active)
  -f, --format=FORMAT     A format string for printing properties and metadata
  -F, --follow            Block and append the query to output when it changes
  -l, --list-all          List the ids of all players
  -w, --wait              Block until the event finishes
  -h, --help              Show this help list
  -v, --version           Print program version
```

# TODO

- Packaged for linux distributions
- Windows installer

# Credits

- [likle/cargs](https://github.com/likle/cargs)
- [keifufu/WebNowPlaying-Library](https://github.com/keifufu/WebNowPlaying-Library)
