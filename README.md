# Tetris in C without lib

The project is to build a tetris game, in C, without much libraries. Allowed
the C stdlib and linux system call, and all graphical will be done by using
direct access to linux framebuffer.

# Run

Run `nix run` in a linux terminal (not under X).

Use `A` and `D` to go left/right and `,` or `o` to rotate the piece.

# Current state

It can display the different pieces


![](assets/tetrismino.png)

And now we can play, the different pieces are generated, going down, stacking. But it does not handle user input.


![](assets/current_stacking.png)
Actually, we can play, and score is increased when lines are completely filled.

TODO: add a screenshot
