#!/bin/bash

SESSION_NAME="debug"
tmux new-session -d -s $SESSION_NAME
tmux split-window -v

tmux send-keys -t $SESSION_NAME:0.0 'wish tcl/entry.tcl' C-m
tmux send-keys -t $SESSION_NAME:0.1 './a.out' C-m

tmux attach -t $SESSION_NAME