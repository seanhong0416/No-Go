/*
Agent with Monte Carlo Tree Search
*/

#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <fstream>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "MCTS.h"

void Node::new_kid(board b_in, board::piece_type who_in){
    //create a new node
    Node node(b_in, who_in);
    //append the new node to kids
    kids.push_back(node);
    
    return;
}