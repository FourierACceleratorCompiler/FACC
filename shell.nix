{pkgs ? import<nixpkgs> {}}:

with pkgs;
mkShell {
	buildInputs = [
		bash ncurses
		cloc ocaml gdb valgrind splint linuxPackages-libre.perf gnumake ctags bc
		# Ocaml packages
		ocamlPackages.findlib ocamlPackages.ocamlbuild ocamlPackages.yojson
		ocamlPackages.core_kernel opam ocamlPackages.cmdliner
		ocamlPackages.ppx_expect ocamlPackages.parmap
		ocamlPackages.alcotest ocamlPackages.core
		# C++ deps for the synnthesizer
		gcc11 nlohmann_json clang
		# Other indirect deps for the synthesizer.
		parallel
		# Deps for tests
		fftw fftwFloat
		# Deps for helper scripts
		python39 python39Packages.numpy python39Packages.matplotlib
		cmake ninja
		# Deps for other experiments
		(callPackage ./decard.nix {})
	];
	# Disable leak sanitizer
	LSAN_OPTIONS="detect_leaks=0";
	# Enable ocaml stack traces.
	OCAMLRUNPARAM = "b";
	SHELL_NAME = "FFTSynth";
	shellHook = ''
			# Don't use rosette
			# raco pkg install rosette
		'';
}
