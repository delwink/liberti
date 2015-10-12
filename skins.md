LiberTI Skins
=============

Skins are an important feature of LiberTI, allowing users to easily change the
appearance of the application requiring no additional programming. Skins are
loaded using libconfig which has its own syntax for defining types.

Skins are loaded from *specification files*, which have a standard extension of
`.lbtskin`. While this extension is preferred for standardization reasons, any
extension (or no extension) will still work with the LiberTI software.

A skin is composed of two major parts: screens and buttons.

Screens
-------

Screens are used for output. It's how the user sees the result of their
calculations, graphs, and plots. It's also how the user can see the cursor and
what characters they have already typed.

Each skin ideally will have at least one screen, but it is permitted to go
without one. It is also possible to have multiple screens in a skin so that the
user can do something such as type a graph and watch it draw at the same time
(though this might not work well on low-end hardware).

The default skin is just a single screen, and it is defined as:

```
screens = [ { mode = "command";
	      x = 0L;
	      y = 0L; } ];
```

Each screen is a `group` element of the `screens` array at the root of the
file. Each group must have the three members `mode`, `x`, and `y`.

The `mode` string specifies the default mode of the screen, but the mode can be
changed through button input (described below). The currently-supported mode
strings are as follows:

- `command` - The standard command mode which takes expression input and
outputs the evaluation of the expression.

The `x` and `y` members of each screen represent coordinates at which the
screen will be placed in the main LiberTI window when this skin is loaded. For
the best future-proofing, these numbers are 64-bit integers. The trailing `L`
is optional with version 1.5 or greater of libconfig, but they are left in this
document for backwards compatibility (Ubuntu 14.04 uses libconfig 1.4.9). As a
skin creator, it is advisable to use the compatible form with the trailing `L`
if you plan on distributing your skin to other users.

Buttons
-------

Buttons are slightly more complex than screens, because their entire behavior
must be defined in the skin specification file.

Each button has a position and a set of actions based on the state of the
LiberTI calculator.

More details coming soon...
