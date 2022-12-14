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

Node * MCTS::selection(Node * parent){
    std::vector<float> v(parent->kids.size());
    //get the value for each child node
    for(int i=0;i<parent->kids.size();i++){
        //if child has not been traversed before
        if(parent->kids[i]->nb == 0){
            return parent->kids[i];
        }
        else{
            v[i] = (float)(-parent->kids[i]->value)/parent->kids[i]->nb + sqrt(2*log(parent->nb)/parent->kids[i]->nb);
        }
    }
    //get the index of the maximum 
    int max_index = -1;
    float max_value = -std::numeric_limits<float>::infinity();
    for(int i=0;i<v.size();i++){
        //printf("index : %d, v_vector : %f, value : %d, nb : %d, parent nb : %d\n",i,v[i], parent->kids[i]->value, parent->kids[i]->nb, parent->nb);
        if(v[i] > max_value){
            max_index = i;
            max_value = v[i];
        }    
    }
    if(max_index == -1) printf("error when selecting max child\n");

    //printf("return index : %d\n",max_index);
    return parent->kids[max_index];
}

int MCTS::random_simulation(Node * start, board::piece_type who_start){
    board current_board = start->b;
    board trial = start->b;
    board::piece_type who_current = who_start;
    std::vector<action::place> space_white(board::size_x * board::size_y);
    std::vector<action::place> space_black(board::size_x * board::size_y);
    for (size_t i = 0; i < space.size(); i++){
        space_white[i] = action::place(i, board::piece_type::white);
        space_black[i] = action::place(i, board::piece_type::black);
    }
    std::vector<board> possible_boards;

    while(1){
        //initialize
        //find all possible outcome boards
        if(who_current == board::piece_type::white){
            for (const action::place& move : space_white) {
                board trial = current_board;
                if (move.apply(trial) == board::legal)
                    possible_boards.push_back(trial);
            }
        }
        else{
            for (const action::place& move : space_black) {
                board trial = current_board;
                if (move.apply(trial) == board::legal)
                    possible_boards.push_back(trial);
            }
        }

        //check if it is the end of the game
        if(possible_boards.size() == 0){
            if(who_current == who_start){
                return -1;
            }
            else return 1;
        }
        else{
            //randomly pick one action to proceed
            std::shuffle(possible_boards.begin(), possible_boards.end(), engine);
            current_board = possible_boards[0];
            trial = current_board;
            possible_boards.clear();
            if(who_current == board::piece_type::white) who_current = board::piece_type::black;
            else who_current = board::piece_type::white;
        }
    }
}

void MCTS::update_value(std::vector<Node *>& route, int v){
    int route_len = route.size();
    for(int i=route_len-2;i>=0;i--){
        route[i]->value = route[i]->value + v;
        route[i]->nb = route[i]->nb + 1;
        v = -v;
    }
}

void Node::new_kids(){
    //need to know the color of player of next state
    board::piece_type next_who;

    //expand all kids by all legal moves
    if(who == board::piece_type::white){
        next_who = board::piece_type::black;
        std::vector<action::place> space_white(board::size_x * board::size_y);
        for (size_t i = 0; i < space_white.size(); i++){
            space_white[i] = action::place(i, board::piece_type::white);
        }
        //printf("enter check illegal move white\n");
        for (const action::place& move : space_white) {
            board after = b;
            if (move.apply(after) == board::legal)
                kids.push_back(new Node(after, next_who, move));
        }
    }
    else{
        next_who = board::piece_type::white;
        std::vector<action::place> space_black(board::size_x * board::size_y);
        for (size_t i = 0; i < space_black.size(); i++){
            space_black[i] = action::place(i, board::piece_type::black);
        }
        //printf("enter check illegal move black\n");
        for (const action::place& move : space_black) {
            board after = b;
            if (move.apply(after) == board::legal)
                kids.push_back(new Node(after, next_who, move));
        }
    }
    
    //printf("size of kids : %d\n",kids.size());

    return;
}

