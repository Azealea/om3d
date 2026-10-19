{
  description = "om3d";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
      config.allowUnfree = true;
    };
  in {
    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        cmake
        gnumake
        gcc
        pkg-config
        renderdoc

        libGL

        libx11
        libxrandr
        libxinerama
        libxcursor
        libxi
        libxext

        wayland
        wayland-scanner
        wayland-protocols
        libxkbcommon
      ];

      LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (with pkgs; [
        libGL
        libx11
        libxrandr
        libxinerama
        libxcursor
        libxi
        libxext
        wayland
        libxkbcommon
      ]);

      shellHook = ''
        echo YAAA
      '';
    };
  };
}
