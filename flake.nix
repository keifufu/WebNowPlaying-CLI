{
  description = "WebNowPlaying-CLI";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    libwnp.url = "github:keifufu/WebNowPlaying-Library";
  };
  outputs = inputs@{ self, flake-parts, libwnp, ... }: 
  flake-parts.lib.mkFlake { inherit inputs; } {
    systems = [
      "x86_64-linux"
      "aarch64-linux"
      "aarch64-darwin"
    ];
    perSystem = { pkgs, system, lib, ...}: let 
      libwnppkg = libwnp.packages.${system}.default;
    in {
      packages.default = pkgs.stdenv.mkDerivation {
        pname = "wnpcli";
        version = builtins.readFile ./VERSION;
        src = ./.;
        nativeBuildInputs = [ pkgs.cmake ]
          ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux [ pkgs.pkg-config ];
        buildInputs = [ libwnppkg ]
          ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux [ pkgs.glib ];
        meta.mainProgram = "wnpcli";
      };
      devShells.default = pkgs.mkShell {
        shellHook = "exec $SHELL";
        buildInputs = with pkgs; [ clang cmake glib pkg-config dpkg rpm ] ++ [ libwnppkg ];
      };
    };
    flake = {
      homeManagerModules.wnpcli = { config, pkgs, lib, ... }: let
        inherit (pkgs.stdenv.hostPlatform) system;
      in {
        options.programs.wnpcli = {
          enable = lib.mkOption {
            type = lib.types.bool;
            default = false;
            description = "Install wnpcli package";
          };
          package = lib.mkOption {
            type = lib.types.package;
            default = self.packages.${system}.default;
            description = "wnpcli package to install";
          };
          service.enable = lib.mkOption {
            type = lib.types.bool;
            default = false;
            description = "Enable wnpcli service";
          };
        };
        config = lib.mkIf config.programs.wnpcli.enable {
          home.packages = [ config.programs.wnpcli.package ];
          systemd.user.services.wnpcli = lib.mkIf config.programs.wnpcli.service.enable {
            Unit = {
              Description = "wnpcli service";
              After = [ "default.target" ];
            };
            Service = {
              Type = "simple";
              ExecStart = "${config.programs.wnpcli.package}/bin/wnpcli start-daemon --no-detach";
              Restart = "always";
            };
            Install.WantedBy = [ "default.target" ];
          };
        };
      };
    };
  };
}
