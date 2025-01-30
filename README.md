# Erodr
Erodr is an implementation of Hans Theobald Beyer's algorithm for simulated hydraulic erosion of landscapes. The algorithm is described in detail in his his thesis *Implementation of a method for hydraulic erosion* which can be found [here](https://ardordeosis.github.io/implementation-of-a-method-for-hydraulic-erosion/thesis-beyer.pdf). Below is the before & after of a 512x512 landscape eroded using Erodr.

![alt text](https://i.gyazo.com/c1b0deb140a5d156bddc0780979f32cd.png)

# Building

## linux
To build for linux simply run `make` (equivalent to `make linux-omp`), `make linux`, or `make linux-omp`.

You can also build a statically linked binary using musl libc by running `make linux-musl`.

## windows
To build for windows (requires mingw-w64) run `make windows` or `make windows-omp`. 

## Note on openmp
Targets with the `-omp` suffix utilize OpenMP to parallelize the algorithm and should run much faster. Keep in mind that the OpenMP implementation of Erodr hasn't been thoroughly tested. If you experience any odd issues or bugs use the standard single-threaded version.

# Usage
```
Usage: erodr -f file [-options]
Simulation options:
    -n ##             Number of particles to simulate (default: 70'000)
    -t ##             Maximum lifetime of a particle (default: 30)
    -g ##             Gravitational constant (default: 4)
    -r ##             Particle erosion radius (default: 2)
    -e ##             Particle inertia coefficient (default: 0.1)
    -c ##             Particle capacity coefficient (default: 10)
    -v ##             Particle evaporation rate (default: 0.1)
    -s ##             Particle erosion coefficient (default: 0.1)
    -d ##             Particle deposition coefficient (default: 1.0)
    -m ##             Minimum slope (default: 0.0001)
Other Options:
    -p <ini-file>     Use provided parameter ini file. See examples/params.ini for an example.
    -o <file>         Place the output into <file>
    -a                Output is ASCII encoded
```

For input and output, Erodr so far only deals with grayscale heightmaps in Netpbm grayscale image format, i.e. \*.pgm files. With no other specified options except the input file (with -f), Erodr will run a simulation with the default parameters (listed above) and output the result to *output.pgm*.

Note: The default parameters listed above have been tuned for a 512x512 heightmap.

## Example usage
Run Erodr on input heightmap `examples/heightmap` using the default simulation parameters:
```
$ ./erodr -f examples/heightmap.pgm
```

Run Erodr on input heightmap `examples/heightmap` provided with the simulation parameters in `examples/params.ini`:
```
$ ./erodr -f examples/heightmap.pgm -p examples/params.ini
```

As above, but use Earth gravity:
```
$ ./erodr -f examples/heightmap.pgm -p examples/params.ini -g 9.81
```

# Contribution
This project was written entirely for fun. If anyone wants to contribute feel free to create a pull request.

# Keep in mind
The software is provided "as is".

[![works badge](https://cdn.jsdelivr.net/gh/nikku/works-on-my-machine@v0.2.0/badge.svg)](https://github.com/nikku/works-on-my-machine)
