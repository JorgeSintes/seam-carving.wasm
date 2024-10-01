# seam-carving.wasm

The idea is to create a minimal webUI that lets us take/use a picture and run the
[seam-carving algorithm](https://en.wikipedia.org/wiki/Seam_carving) on it, selecting the axis to shrink and
by how much.

- The algorithm implementation will be done in C, compiled to WASM.
- The frontend will likely be built in Vue and tailwind.
