Mish — A minimal linux shell written in C.

Why another shell? I wanted a minimal clean and simple shell. It has all the features most users would use and gets its done in a tiny package.

![image](https://user-images.githubusercontent.com/94549325/205668858-a7c8850c-1bfe-4776-a5fe-6adc3195629b.png)

# Install
1. Clone the git-repo with
```
git clone https://github.com/vnnm404/mish
```

2. `cd` into the mish directory created
```
cd mish
```

3. run `make`
```
make
```

# Usage
To start the shell, simply run the executable created by running `make`.
```
./sh
```

The directory in which the shell is spawned is considered the `$HOME` directory and thats what `~` refers to.

# Features
Has all the good stuff a modern shell would have, minimal bash autocompletion with `<Tab>`, piping and redirection, job control and foreground-background processes, ctrl-c and ctrl-z implementations and command history. In short it has everything most people would need out of a shell and all in 56kb!
