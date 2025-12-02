
# TODO list

This is me keeping track of my progress throughout the project. The following won't necessarly be done.
Let's maybe (re)start with this:
* [ ] Make documentation (brief, param, return, throw, and example)
  * [X] move.hpp
  * [X] positionBase.hpp
  * [X] boardUI.hpp
* [ ] Put stuff in cpp files rather than hpp

and then we will continue the development:

* [X] Make a chess UI
  * [X] print the board in the console
  * [X] handle moves
  * [X] handle FEN
  * [X] enable marking of squares
* [ ] Include Stockfish
  * [X] create a Wrapper for uci communication
  * [X] create an equivalent for human player
  * [ ] figure out how to ake stockfish evaluate a position
* [ ] Move generation
  * [ ] implement the general structure (base classes)
  * [ ] implement naive move generation
  * [ ] implement Z keys
  * [ ] improve move generation with pins and stuff like that
  * [ ] implement magic billboards
* [ ] Testing the software
  * [ ] look at move generation on board
  * [ ] check number of positions
  * [ ] check move generation performance
* [ ] Search algorithm
  * [ ] implement alpha beta pruning
  * [ ] implement search through captures
  * [ ] implement incremental search
  * [ ] implement move ordering
  * [ ] implement extensions
* [ ] Evaluation function
  * [ ] implement basic evaluation
  * [ ] export all evaluated positions and create database with evaluations from stockfish
  * [ ] train AI on this
* [ ] Arena
  * [ ] add initial performance tests in the Arena
  * [ ] create database of equal positions for starting positions in Arena
  * [ ] confront engines on different times
  * [ ] generate png files for games
* [ ] Publishing
  * [ ] handle time control
  * [ ] implement full UCI communication
* [ ] Reinfiorcement learning
  * [ ] create trainable parameters
  * [ ] train
* [ ] Get a state of the art engine
  * [ ] look at recent advancements in the field
  * [ ] try to implement some