# TODO: Double check which are actually needed for current dependencies
DEPENDENCIES="libfreetype-dev"

PHONY: debug
debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
	make -C build nodeeditor

PHONY: test
test:
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
	make -C build tests

PHONY: install
install:
	sudo apt install ${DEPENDENCIES}
	cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
	make -C build nodeeditor

PHONY: run
run:
	./build/src/nodeeditor
