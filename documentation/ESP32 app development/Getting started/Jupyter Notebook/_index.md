---
title: "Jupyter Notebook"
nodateline: true
weight: 5
---

When coding the badge in (micro)python, it can be useful to use a
[Jupyter Notebook](https://jupyter.org). This allows you to keep a
'scratch pad' of code snippets that you can easily and quickly adapt
and run on the badge, without having to manually copy-paste code between your
editor and the REPL all the time.

Normally a Jupyter Notebook would run the python code on your development
machine. To make it run the code on your badge instead, you use the
[Jupyter MicroPython Kernel](https://github.com/goatchurchprime/jupyter_micropython_kernel).

You can see a quick video of a notebook in action
[here](https://arnout.engelen.eu/micropython.mp4).

# Installation

This setup works best with Python 3. The easiest way to install is to create a
virtualenv:

```
~$ mkdir badgehacking
~$ cd badgehacking
~/badgehacking$ python3 -m venv environment
~/badgehacking$ source environment/bin/activate
```

Install jupyter:

```
~/badgehacking$ pip install jupyter
```

Download and install the Jupyter MicroPython Kernel:

```
~/badgehacking$ git clone https://github.com/goatchurchprime/jupyter_micropython_kernel.git
~/badgehacking$ pip install -e jupyter_micropython_kernel
~/badgehacking$ python -m jupyter_micropython_kernel.install
```

If all went well, jupyter should now show micropython in the list of available
kernels:

```
~/badgehacking$ jupyter kernelspec list
Available kernels:
  micropython    /home/aengelen/.local/share/jupyter/kernels/micropython
  python3        /home/aengelen/badgehacking/environment/share/jupyter/kernels/python3
```

# Usage

To start the notebook, first enter the virtualenv again:

```
~$ cd badgehacking
~/badgehacking$ source environment/bin/activate
```

Start Jupyter:

```
~/badgehacking$ jupyter notebook
```

This should start the jupyter server on your machine, and open a browser window
to interact with it. In that browser window, choose 'New...' and select
'MicroPython - USB'. This will open a new MicroPython-enabled Notebook.

This will show a page with a 'block' that accepts python code. You can use
Ctrl+Enter to execute the code in the block, and Alt+Enter to create a new
block.

Before you can execute any commands, you will need to connect the notebook to
your badge via the serial bus by adding the special command `%serialconnect`
to a block and executing it. When you see `Ready.` the connection was
succesful. On some badges you need to issue this command twice.

# Limitations

Currently, a disadvantage of the Jupyter Notebook over using the REPL directly
is that code completion (tab completion) is not yet supported in the Jupyter
MicroPython Kernel. Jupyter does support completion with other kernels, so it
is likely possible to add this feature in the future.

# Links

The [documentation for the Jupyter MicroPython Kernel](https://github.com/goatchurchprime/jupyter_micropython_kernel)
is quite good.
