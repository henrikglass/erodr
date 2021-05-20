# Erodr
Erodr is an implementation of Hans Theobald Beyer's algorithm for simulated hydraulic erosion of landscapes. The algorithm is described in detail in his his thesis *Implementation of a method for hydraulic erosion* which can be found [here](https://www.firespark.de/resources/downloads/implementation%20of%20a%20methode%20for%20hydraulic%20erosion.pdf). Below is the before & after of a 512x512 landscape eroded using Erodr.

![alt text](https://i.gyazo.com/c1b0deb140a5d156bddc0780979f32cd.png)

# Building
To build, simply run `make` in the root of the repository. I recommend to play around a bit with the optimization flags under CFLAGS in the makefile. On some machines "-O1" produced a faster result than both "-O2" and "-O3". Enabling "-march=native" may improve performance on some machines.

# Usage
```
Usage: erodr -f file [-options]
Simulation options:
    -n ##             Number of particles to simulate (default: 70'000)
    -t ##             Maximum lifetime of a particle (default: 30)
    -g ##             Gravitational constant (default: 4)
    -r ##             Particle erosion radius (default: 2)
    -e ##             Particle enertia coefficient (default: 0.1)
    -c ##             Particle capacity coefficient (default: 10)
    -v ##             Particle evaporation rate (default: 0.1)
    -s ##             Particle erosion coefficient (default: 0.1)
    -d ##             Particle deposition coefficient (default: 1.0)
    -m ##             Minimum slope (default: 0.0001)
Other Options:
    -o <file>         Place the output into <file>
    -a                Output is ASCII encoded
```

For input and output, Erodr so far only deals with grayscale heightmaps in Netpbm grayscale image format, i.e. \*.pgm files. With no other specified options except the input file (with -f), Erodr will run a simulation with the default parameters (listed above) and output the result to *output.pgm*.

# A known issue
Input images that contains completely black areas can cause erodr to produce spike artifacts. This is because any erosion below the minimum value (0) simply wraps around to the maximum value. Why isn't this fixed? Because I'm lazy. Also, because there is a simple workaround; just raise the floor a bit. If you experience this issue, open your image in an image editor and lower the contrast slightly before using it with erodr. Alternatively, feel free fix the issue yourself and create a pull request.

# Contribution
This project was written entirely for fun. If anyone wants to contribute feel free to create a pull request.

# Keep in mind
The software is provided "as is".

[![works badge](https://cdn.jsdelivr.net/gh/nikku/works-on-my-machine@v0.2.0/badge.svg)](https://github.com/nikku/works-on-my-machine)
