{
  inputs = {
    nixpkgs.url = "git+https://github.com/NixOS/nixpkgs?shallow=1&ref=nixos-unstable";
  };

  outputs = { nixpkgs, ... }: {
    devShells.x86_64-linux.default = with nixpkgs.legacyPackages.x86_64-linux; mkShell {
      buildInputs = [ gcc ccls ];
    };
    packages.x86_64-linux.default = with nixpkgs.legacyPackages.x86_64-linux; stdenv.mkDerivation
      {
        pname = "tetris";
        version = "0.0.0";
        src = ./.;
        buildInputs = [ gnumake ];
        buildPhase = ''
          mkdir -p $out/bin
          make tetris
        '';
        installPhase = ''
          cp tetris $out/bin/
        '';
      };
  };
}
