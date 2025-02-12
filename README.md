# Erodr
Erodr is an implementation of Hans Theobald Beyer's algorithm for simulated hydraulic erosion of landscapes. The algorithm is described in detail in his his thesis *Implementation of a method for hydraulic erosion* which can be found [here](https://ardordeosis.github.io/implementation-of-a-method-for-hydraulic-erosion/thesis-beyer.pdf).

![erodr.png](https://github.com/henrikglass/erodr/blob/master/examples/erodr.png?raw=true)
This image shows a 1024x1024 heightmap after being processed inside the Erodr visualizer/live preview window.

Link to a video demo of Erodr: https://www.youtube.com/watch?v=jiTiTiV2hcU

## Changes in version v2.0.0
### Visualizer/Live preview
Erodr v2.0.0 includes a built-in 3D heightmap live preview (built using [Raylib](https://www.raylib.com/)). The live preview allows you to:

- Watch your heightmap transform in real time as you are running the particle simulation.
- Reload parameters from a parameter ini-file and re-run the simulation without closing the program.
- View the generated terrain using a handful of different visualizer modes (including a snow cover visualizer, gradient visualizer, pseudo coloring based on height, etc.)

The visualizer/live preview is launched by default. If you wish to run Erodr without the visualizer you can supply erodr with the `--no-ui` option, which makes erodr behave like in previous versions.

### More tunable parameters
Erodr v2.0.0 includes more tunable parameters:

- `-f` or `--initial-velocity`. This option specifies the initial velocity of the particle. In previous versions of Erodr this value was implicitly set to 0. I found out that using a value closer to 1 tends to produce better looking results.
- `-w` or `--initial-water`. This option specifies the initial water content of the particle. In previous versions of Erodr this value was implicitly set to 1.

### GEPT ([GE]neric [P]rogrammable [T]emplates)
Erodr is now bundled with gept ([GE]neric [P]rogrammable [T]emplates). Gept is a program I developed to add some semblance of meta-programming support to C and other languages that don't have compile time execution. For Erodr, I use it to embed shader source code directly into the binary. If you wish to build gept on your own, you can get the source code here: https://github.com/henrikglass/gept

### Musl libc version removed
Erodr v2.0.0 does *not* include a musl libc build. Trying to get musl libc to work with raylib was a big pain, so I abandoned it.

If you\'re running on Linux and your glibc version does not match the version required by the prebuilt binary, you should be able to build Erodr yourself without any issue. The only requirement is a C compiler. Here\'s the simplest possible command to compile Erodr for linux (without using the makefile and without openmp support):

```shell
$ tools/gept -i src/shaders/shaders.h.template > src/shaders/shaders.h && cc -Iinclude -Llib/linux src/*.c -lraylib -lm -lpthread -ldl
```

# Building

## linux
To build for linux simply run `make` (equivalent to `make linux-omp`), `make linux`, or `make linux-omp`.

## windows
To build for windows (requires mingw-w64) run `make windows` or `make windows-omp`.

## Note on openmp
Targets with the `-omp` suffix utilize OpenMP to parallelize the algorithm and should run much faster. Keep in mind that the OpenMP implementation of Erodr hasn't been thoroughly tested. If you experience any odd issues or bugs use the standard single-threaded version.

# Usage
```
Usage: erodr [Options]
Options:
  -i,--input                       path to input heightmap *.pgm file (default = (null))
  -o,--output                      path to output heightmap *.pgm file (default = output.pgm)
  -p,--params                      path to simulation parameters *.ini file (default = (null))
  -a, --ascii                      Use ascii encoding for output *.pgm file. (default = 0)
  -n,--num-particles               Number of particles to simulate (default = 70000, valid range = [-9223372036854775808, 9223372036854775807])
  -t,--ttl                         Maximum lifetime of a particle (default = 32, valid range = [-9223372036854775808, 9223372036854775807])
  -r,--radius                      Particle erosion radius (default = 2, valid range = [-9223372036854775808, 9223372036854775807])
  -g,--gravity                     Gravitational constant (default = 1, valid range = [-1.7976931e+308, 1.7976931e+308])
  -y,--inertia                     Particle inertia coefficient (default = 0.3, valid range = [-1.7976931e+308, 1.7976931e+308])
  -c,--capacity                    Particle capacity coefficient (default = 8, valid range = [-1.7976931e+308, 1.7976931e+308])
  -v,--evaporation-rate            Particle evaporation rate (default = 0.02, valid range = [-1.7976931e+308, 1.7976931e+308])
  -s,--erosion-coefficient         Particle erosion coefficient (default = 0.7, valid range = [-1.7976931e+308, 1.7976931e+308])
  -d,--deposition-coefficient      Particle deposition coefficient (default = 0.3, valid range = [-1.7976931e+308, 1.7976931e+308])
  -m,--minimum-slope               Minimum slope (default = 0.0001, valid range = [-1.7976931e+308, 1.7976931e+308])
  -f,--initial-velocity            Particle initial velocity (default = 0.9, valid range = [-1.7976931e+308, 1.7976931e+308])
  -w,--initial-water               Particle initial water content (default = 1, valid range = [-1.7976931e+308, 1.7976931e+308])
  --no-ui                          Don't open the UI/Visualizer (just perform the simulation and save like older versions of erodr did) (default = 0)
  --help                           Show this message (default = 0)
  --generate-completion-cmd        Generate a completion command for Erodr on stdout (default = 0)
```
With no other specified options except the input file (`-i`, or `--input`) Erodr will launch with the default parameters.

For input and output, Erodr so far only supports grayscale heightmaps in Netpbm grayscale image format, i.e. \*.pgm files. 

## Example usage
Run Erodr on input heightmap `examples/heightmap.pgm` using the default simulation parameters:
```
$ ./erodr -i examples/heightmap.pgm
```

Run Erodr on input heightmap `examples/heightmap` provided with the simulation parameters in `examples/params.ini`:
```
$ ./erodr -i examples/heightmap.pgm -p examples/params.ini
```

As above, but use Earth gravity:
```
$ ./erodr -i examples/heightmap.pgm -p examples/params.ini -g 9.81
```

As above, but run Erodr without the visualizer (directly output to `output.pgm`):
```
$ ./erodr -i examples/heightmap.pgm -p examples/params.ini -g 9.81 --no-ui
```

# Contribution
This project was written entirely for fun. If anyone wants to contribute feel free to create a pull request.

# Keep in mind
The software is provided "as is".

[![works badge](https://cdn.jsdelivr.net/gh/nikku/works-on-my-machine@v0.2.0/badge.svg)](https://github.com/nikku/works-on-my-machine)
