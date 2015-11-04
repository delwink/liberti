LiberTI Skins
=============

Skins are an important feature of LiberTI, allowing users to easily change the
appearance of the application requiring no additional programming. Skins are
loaded using libconfig which has its own syntax for defining types.

Skins are searched for in `/usr/share/liberti/skins/` and
`~/.liberti/skins/`. Each subdirectory of these locations is considered to be a
skin. Within that folder is the *specification*, which should be exactly named
`spec.conf`. All relative paths mentioned in this configuration file will be in
relation to the skin's root path.

The background image of the skin is defined in the term `background`, and it
must point to a PNG image. If the `background` term is omitted, a the skin will
be plain white.

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

Each button has a position, a size, and a set of actions based on the state of
the LiberTI calculator. Each is defined as part of an array of buttons, as
such:

```
buttons = [ { x = 10;
              y = 10;
              w = 20;
              h = 20;
              actions = { default = { normal = { type = "shift"; }
                                      shift = { type = "shift"; }
                                      alpha = { type = "shift"; } } } } ];
```

This creates a button at position (10, 10) with a width and height of 20
pixels. This button only defines a single action, which is to toggle the
`shift` action state of the calculator regardless of the current action
state. The possible command modes are the ones listed above for defining the
default mode of a screen, as well as `default` to fill in the gaps for
undefined mode actions (which is useful for an example like above which does
the same thing in multiple/all modes).

### Action States

The calculator is usually in the `normal` action state. This should be the
button action written on the button itself (as on the real physical
calculator), and it is the primary purpose of that button.

When `2nd` mode is active, it is in the `shift` action state. This should be
the secondary action used by the button. For instance, a button which normally
does the sine function might use the inverse sine function as its secondary
action.

The `alpha` action state is used for writing text. A typical button should
insert some character in alpha mode.

### Possible Actions

Each action must have a `type` which defines the kind of action, and some must
have a `which`, defining a more specific action of that type. All `type` and
`which` properties must be strings.

- `mode` - Changes the active screen to `which` mode is selected from the mode
list above.
- `char` - Inserts `which` character to the active screen.
- `move` - Moves the cursor in `which` direction, one of up, down, left, or
right.
- `shift` - Toggles the `shift` action state.
- `alpha` - Toggles the `alpha` action state.
