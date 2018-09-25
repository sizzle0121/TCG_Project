# TCG_Project
`Theory of Computer Game course projects.`  
## [2584_AI-TD learning](https://github.com/sizzle0121/TCG_Project/tree/master/2584-AI-TD)  
## Introduction:  
An AI agent which is able to play 2584 game and reach human-level performance.  
Implement it with Temporal Difference Learning and N-tuple network to extract features of the board.  
  
## Instructions:  
>__To make the sample program__  
:point_right:$ make     #see makefile for deatils  
  
>__To run the sample program__  
:point_right:$ ./2584   #by default the program will run for 1000 games  
  
>__To specify the total games to run__  
:point_right:$ ./2584 --total=100000  
  
>__To save the statistic result to a file__  
:point_right:$ ./2584 --save=stat.bin #existing file will be overwrited  
  
>__To load and review the statistic result from a file__  
:point_right:$ ./2584 --load=stat.bin --summary  
  
>__To display the statistic every 1000 episodes__  
:point_right:$ ./2584 --total=100000 --block=1000  
  
>__To save the weights of player to a file__  
:point_right:$ ./2584 --play="save=weights.bin"  
  
>__To load the weights of player from a file__  
:point_right:$ ./2584 --play="load=weights.bin"  
  
>__To train the network 100000 games, and save the weights to a file__  
:point_right:$ ./2584 --total=100000 --block=1000 --play="save=weights.bin"  
  
>__To load the weights from a file, train the network for 100000 games, and save the weights__  
:point_right:$ ./2584 --total=100000 --block=1000 --play="load=weights.bin save=weights.bin"  
  
>__To set the learning rate of training__  
:point_right:$ ./2584 --play="alpha=0.0025"  
  
>__To load the weights from a file, test the network for 1000 games, and save the statistic__  
:point_right:$ ./2584 --total=1000 --play="load=weights.bin alpha=0" --save="stat.bin"  
