# Boost.Asio Recipes

This repository contains a collection of **recipes** designed to help you become
familiar with the **Boost.Asio** library.
Each recipe gradually increases in complexity and introduces new features of the
library step by step.

These examples are based on the following Packt publication:

> [**Boost.Asio C++ Network Programming Cookbook**](https://www.packtpub.com/en-us/product/boostasio-c-network-programming-cookbook-9781783986552)
> Over 25 hands-on recipes for creating robust, high-performance, cross-platform
> distributed applications with the Boost.Asio library.


## Building the Executables

This project uses the [**SeeMake**](https://github.com/MhmRhm/SeeMake) template.
To build it on **Windows** or **Linux**, you’ll need to install the required
dependencies first.

* **Linux setup:** Follow the instructions [here](https://github.com/MhmRhm/SeeMake?tab=readme-ov-file#setting-up-linux).
* **Windows setup:** Follow the instructions [here](https://github.com/MhmRhm/SeeMake?tab=readme-ov-file#setting-up-windows).

Once the dependencies are installed, you can build all executables using the
following commands:

```bash
# On Linux
cmake --workflow --preset linux-default-release

# On Windows
cmake --workflow --preset windows-clang-release
# or
cmake --workflow --preset windows-default-release
```

## Recipes

Below is a list of the available recipes and what you can learn from each one:
1. **asio1:** Learn how to define IP addresses and endpoints using Boost.Asio.
