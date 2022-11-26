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
#include <cmath>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "MCTS.h"

Node MCTS::selection(Node parent){
    std::vector<float> v(parent.kids.size());
    //get the value for each child node
    for(int i=0;i<parent.kids.size();i++){
        //if child has not been traversed before
        if(parent.kids[i].nb == 0){
            v[i] = std::numeric_limits<float>::infinity();
        }
        else{
            v[i] = -parent.kids[i].value/parent.kids[i].nb + sqrt(2*log(parent.nb)/parent.kids[i].nb);
        }
    }
    //get the index of the maximum 
    int max_index = -1;
    float max_value = -std::numeric_limits<float>::infinity();
    for(int i=0;i<v.size();i++){
        if(v[i] > max_value){
            max_index = i;
            max_value = v[i];
        }    
    }
    if(max_index == -1) printf("error when selecting max child\n");

    return parent.kids[max_index];
}

int MCTS::random_simulation(Node start, board::piece_type who_start){
    board current_board = start.b;
    board trial = start.b;
    while(1){
        //check if current player has any avaliable move

        //randomly pick one action to continue

    }
}

void Node::new_kids(){
    //expand all kids by applying all the possible moves
    for(size_t i=0;i<space.size();i++) {
        space[i] = action::place(i, who);
    }

    //need to know the color of player of next state
    board::piece_type next_who;
    if(who == board::piece_type::black) next_who = board::piece_type::white;
    else if(who == board::piece_type::white) next_who = board::piece_type::black;
    else printf("error when choosing next_who\n");

    //expand all kids by all legal moves
    for (const action::place& move : space) {
        board after = b;
        if (move.apply(after) == board::legal)
            kids.push_back(Node(after, next_who));
    }
    
    return;
}