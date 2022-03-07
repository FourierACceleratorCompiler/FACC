{ stdenv, fetchFromGitHub, gcc, gnumake, flex, bison, python27 }:

stdenv.mkDerivation {
	name = "deckard";
	src = fetchFromGitHub {
		repo = "Deckard";
		owner = "skyhover";
		rev = "24164dc";
		sha256 = "sha256:0734vzcb2w044kdvb5fdcs4nsqlmny4qair1p49vr62l0h8a177n";
	};

	buildInputs = [
		gcc
		gnumake flex bison python27
	];

	configurePhase = ''
		patchShebangs ./src/ptgen/
		cat ./src/ptgen/gcc/mainc.py
		'';

	buildPhase = ''
		cd src/main
		./build.sh
		cd ../..
		'';
	installPhase = ''
		mkdir -p $out/bin
		ls src
		ls src/main
		cp src/main/cvecgen $out/bin
		'';
}
