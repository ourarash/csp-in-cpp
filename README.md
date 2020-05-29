# Golang and CSP-like channels for C++

> Can be used in Visual Studio Code

Features:

- [x] Supports CSP channels and Read and Write operations.

## Installation

```bash
git clone https://github.com/ourarash/csp-in-cpp.git
```

### Run main:

You can run this using `blaze`:

```bash
bazel run src/main:main
```

### Run Tests:

You can run unit tests using [`blaze`](installing-bazel):

```bash
bazel test tests:tests
```


## Installing Bazel

This repo uses `Bazel` for building C++ files.
You can install Bazel using this [link](https://docs.bazel.build/versions/master/install.html).

## Debugging with Bazel

There are two configurations available: `(lldb) launch` and `CodeLLDB`. You can use `(lldb) launch` without any modifications, but Currently only `CodeLLDB` provides correct pretty printing for STL containers such as map and vector.

### Using CodeLLDB

<img alt="Directory Structure" src="https://github.com/ourarash/cpp-template/blob/master/codelldb1.png?raw=true" width="400">

In order for CodeLLDB to work with Bazel on Visual studio code and provide pretty printing, you need the following:

- Install [CodeLLDB Extension](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)

<img alt="Directory Structure" src="https://github.com/ourarash/cpp-template/blob/master/codelldb2.png?raw=true" width="400">

- Run this command to create Bazel symlinks:

```bash
bazel build src/main:main
```

- Run one of the following commands depending on your system (copied from [launch.json](.vscode/launch.json)) to build with bazel for debug.

```bash
"Linux": "bazel  build --cxxopt='-std=c++17' src/main:main -c dbg",
"windows": "bazel build --cxxopt='-std=c++17' src/main:main --experimental_enable_runfiles -c dbg"
"mac":"command": "bazel build --cxxopt='-std=c++17' src/main:main -c dbg --spawn_strategy=standalone"
```

- Run this in the root of your workspace to find the target of `bazel-cpp-template` symlink that Bazel creates based. These symlinks are documented [here](https://docs.bazel.build/versions/master/output_directories.html):

```bash
readlink -n bazel-cpp-template
```

- Put the output of that command in [launch.json](.vscode/launch.json)'s sourcemap section:

```json
"sourceMap": {
        "[output of readlink -n bazel-cpp-template]": "${workspaceFolder}/"
 }
```

Example:

```json
"sourceMap": {
        "/private/var/tmp/_bazel_ari/asdfasdfasdfasdfasdfgadfgasdg/execroot/__main__": "${workspaceFolder}/"
 }
```

- Start debugging!

Here is a video that explains more about how to use Visual Studio Code for debugging C++ programs:

<table><tr><td>

<a href="https://www.youtube.com/watch?v=-TUogVOs1Qg/">
<img alt="Debugging C++ in Visual Studio Code using gcc/gdb and Bazel" src="https://raw.githubusercontent.com/ourarash/cpp-template/master/bazel_yt.png" width="400">
</a>
</td></tr></table>

# More on using Google Test with Bazel in Visual Studio Code:

Here is a video that explains more about how to use Google Test with Bazel in Visual Studio Code:

<table><tr><td>

<a href="https://www.youtube.com/watch?v=0wMNtl2xDT0/">
<img border="5" alt="Debugging C++ in Visual Studio Code using gcc/gdb and Bazel" src="https://raw.githubusercontent.com/ourarash/cpp-template/master/VSCDebug_yt.png" width="400">
</a>
</td></tr></table>

# More Info On Debugging in VCS:

Check this [page](https://code.visualstudio.com/docs/cpp/cpp-debug).
