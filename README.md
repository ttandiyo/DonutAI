# DonutAI

A cross-platform C++ implementation of a bot player for the game of Connect 4. 

## Prerequisites
- GCC that supports C++14 or later
- Java (to run GUI shell)

## Usage
### Compile binary only
`make` will compile a binary in the `/bin` folder. 

### Play against DonutAI
`make run` will compile a binary if it doesn't exist yet, and launch the GUI shell to let you play Connect 4 against DonutAI.

#### Customize game rules
If you'd like, you can press `File > New` in the GUI shell and adjust the game rules. DonutAI will try its best to adapt to your rules!
- **Width/Height**: Width and height of gameboard
- **K**: Number of pieces in a row (vertically, horizontally, or diagonally) required to win
- **Gravity**: Whether the placed pieces fall to the bottom of the board or not

# History

DonutAI was written by me as part of a university project for a tournament which pitted every participant's bot player against the others'. Each bot player must communicate state with the provided shell using its API and is allotted 5 seconds to make a move. DonutAI placed in the top 20%. A large amount of time on this project was devoted to profiling the code and improving its performance. Its core strategy is to maximize for itself and minimize for the opponent the possible number of ways to win. The algorithm uses iterative deepening searching (IDS) to evaluate positions 1 move away, then 2, and so on. Alpha-beta pruning was employed to dramatically increase search efficiency, increasing the maximum search depth by around 2 levels. 

Various improvements to the player were considered, such as with weighted heuristics and multithreading, but were dropped because of time constraints. Simple effective heuristics could increase the effective search depth by around 1.5 levels (see [Strategy](#strategy)).

I would like to try making a DonutAI 2 using ML whenever I have time!

# Strategy
In the game of ConnectK, it is trivial to create a computer player that can defend against immediately visible losses and end games in a draw against naive players by simply brute-forcing the game tree and looking for wins/losses, without considering the longer-term implications of certain moves and formations. However, when there are no immediately visible wins or losses within the horizon, it takes additional work to have the "foresight" to plan significantly ahead and either bring out a forced win or avoid a forced loss.

A simple heuristic evaluation function can not accurately determine the relative utility of different states, often assigning the same value to two states when, to a human, they are obviously different in value.

A computer player that can recognize these nuances in the game will be able to provide significantly more *intelligent* gameplay. This is especially important for ConnectK's *gravity disabled* mode, whose game tree branches by the number of empty spaces available, which decreases by one at each level, and so has $n!$ nodes to evaluate, where $n$ is the number of empty spaces remaining in the actual board. The default board size of $9\times7$ thus has an impractically large complete search space of $63!$—increasing the effective search depth as much as possible along with making generally beneficial moves whenever possible is crucial for making a formidable computer ConnectK player.

The main avenue for accomplishing this is through the study of the game and incorporation of specific knowledge of the game into a static evaluator function. *(With a more expensive evaluator function, it may be worth implementing a hash table for score lookup to avoid having to compute the score for a state more than once, considering that there are a very large number of identical states which can be reached simply by a different ordering of moves, as well as rotated and/or mirrored equivalent states.)*

## Guaranteed wins
*Assuming $k = 5$ and gravity disabled*

Besides the obvious guaranteed win of *5 pieces in a row*, in order to increase its effective lookahead ability, the evaluator should recognize and appropriately score *traps*, which are scenarios that are guaranteed to result in winning or losing after an additional turn, as second only to an actual win or loss.

A *trap* is an instance of either:
- 2 or more lines of 4 pieces in a row which each have their own empty space available
- 4 pieces in a row which have an empty space on either side

These are the only scenarios in which a computer player, non-error-prone unlike human players, can lose.

If these game-ending scenarios are accounted for and detected in the evaluator, the effective search depth for finding a winner/loser is increased by *2 levels* compared to naively only considering 5 pieces in a row as winning, which is a large improvement.

## Thinking ahead
Additionally, the evaluator can *try to recognize the steps leading up to these scenarios*. To this end, a simple improvement can be considered where the evaluator should be recognizing instances of:
- 4 pieces in a row and 1 empty space on each side (a win)
- 2 lines of (3 pieces in a row and 2 empty spaces) which share an empty space (a win)
- 4 pieces in a row and 1 empty space
- 3 pieces in a row and 2 empty spaces
- 2 pieces in a row and 3 empty spaces
- 1 piece in a row and 4 empty spaces
- 5 empty spaces

#### Weighting
These should contribute to the state score in descending significance. The weighting of these factors can be initially decided with some intuitive relative comparisons.

How important or valuable is

- 4 pieces in a row and 1 empty space

compared to

- 3 pieces in a row and 2 empty spaces?

Intuitively speaking, it seems reasonable to say that the first situation is around twice as valuable as the second. The weights can be tuned with empirical data at a later point.

## Ordering
Equally as important as, or possibly more important than, how many pieces are in a given frame is the ordering of said pieces.


`_OOO__` or `__OOO_` is equally as valuable as `OOOO_` or `_OOOO` or `OO_OO` or other combinations of *4 pieces/1 empty* because both situations force an immediate block from the other player.

It is worth noting that the above situation is not exactly *3 pieces/2 empty*, and is probably better described as *3 pieces in a row/3+ empty*. Any other combination of *3 pieces/2 empty* seems to be less valuable than any combination of *4 pieces/1 empty*. None of these other combinations requires an immediate response from the other player:

`OOO__`, `O_O_O`, `OO_O_`, `OO__O`, `O_OO_`, . . .

Handling all the different different combinations of pieces after this point gets slightly more involved. It would be beneficial to further generalize the rules—the most desirable formations are probably contiguous blocks of pieces, while all other formations with the same number of pieces in a frame are all worth some smaller amount.

For instance, `_OO__` is probably more valuable than `O__O_`.

### Generalization
After thinking through some situations given a $k$ of $5$, some general rules can be made that the static evaluation should attempt to embody, if the increased computation cost incurred by their implementation is outweighed by a resulting improvement in playing performance.
1. $k$ pieces (frame of $k$): *win*
2. $(k - 1)$ pieces / $1$ space per end (frame of $(k+1)$): *win - 1*
3. multiple $(k - 1)$ pieces / $1$ space (frames of $k$): *win - 1*
    - case 2 is really just a specialization of this case
    - may be possible to create algorithm to cover both cases
4. $(k - 1)$ pieces / $1$ space (frame of $k$): $c$
5. $(k - 2)$ pieces in center / $3$ spaces total (frame of $(k + 1)$): $c$
6. $(k - 2)$ pieces noncentered / $2$ spaces (frame of $k$): $c/2$
7. $(k - 3)$ pieces / $3$ spaces (frame of $k$): $c/4$
8. ...
