{
  inputs = {
    nixpkgs.url = "git+https://github.com/NixOS/nixpkgs?shallow=1&ref=nixos-unstable";
  };

  outputs = { nixpkgs, ... }: {
    devShells.x86_64-linux.default = with nixpkgs.legacyPackages.x86_64-linux; mkShell {
      buildInputs = [ clang ccls ];
    };
    packages.x86_64-linux.default = with nixpkgs.legacyPackages.x86_64-linux; runCommand "tetris"
      {
        buildInputs = [ clang ];
      }
      ''
        mkdir -p $out/bin
        clang -O3 ${./tetris.c} -o $out/bin/tetris;
      '';
  };
}
