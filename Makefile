# TODO: Double check which are actually needed for current dependencies
DEPENDENCIES="libfreetype-dev"

PHONY: mapgen
mapgen:
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
	make -C build mapgen

PHONY: test
test:
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
	make -C build tests

PHONY: install
install:
	sudo apt install ${DEPENDENCIES}
	cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
	make -C build mapgen
