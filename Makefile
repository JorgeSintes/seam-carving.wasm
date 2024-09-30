main: main.c
	clang -O3 -I./libs/ -lm -o main.bin main.c

add: add.c
	clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export-all -o add.wasm add.c

clean:
	rm -f *.bin *.wasm