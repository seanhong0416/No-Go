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
#include "MC_RAVE.h"

Node_RAVE * MC_RAVE::selection(Node_RAVE * parent){
    std::vector<float> v(parent->kids.size());
    float move_mean;
    float RAVE_mean;
    float MC_RAVE_weight;
    //get the value for each child node
    for(int i=0;i<parent->kids.size();i++){
        //if child has not been traversed before
        if(parent->kids[i]->nb == 0){
            return parent->kids[i];
        }
        else{
            move_mean = (float)(-parent->kids[i]->value)/parent->kids[i]->nb;
            RAVE_mean = (float)(-parent->kids[i]->RAVE_value)/parent->kids[i]->RAVE_nb;
            MC_RAVE_weight = sqrt((float)(equivalence_parameter)/(3*parent->nb+equivalence_parameter));
            //if(move_mean != RAVE_mean)
            //    printf("move_mean = %f, RAVE_mean = %f, move_value = %d, move_nb = %d, RAVE_value = %d, RAVE_nb = %d\n", move_mean, RAVE_mean, parent->kids[i]->value, parent->kids[i]->nb, parent->kids[i]->RAVE_value, parent->kids[i]->RAVE_nb);
            //printf("MC_RAVE_WEIGHT:%f\n",MC_RAVE_weight);
            v[i] = (1-MC_RAVE_weight)*move_mean + MC_RAVE_weight*RAVE_mean + sqrt(2*log(parent->nb)/parent->kids[i]->nb);
        }
    }
    //get the index of the maximum 
    int max_index = -1;
    float max_value = -std::numeric_limits<float>::infinity();
    for(int i=0;i<v.size();i++){
        //printf("index : %d, v_vector : %f, value : %d, RAVE_value : %d, nb : %d, RAVE nb : %d, parent nb : %d\n",i,v[i], parent->kids[i]->value, parent->kids[i]->RAVE_value, parent->kids[i]->nb, parent->kids[i]->RAVE_nb , parent->nb);
        if(v[i] > max_value){
            max_index = i;
            max_value = v[i];
        }    
    }
    if(max_index == -1) printf("error when selecting max child\n");

    //printf("return index : %d\n",max_index);
    return parent->kids[max_index];
}

int MC_RAVE::random_simulation(Node_RAVE * start, board::piece_type who_start){
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

void MC_RAVE::update_value(std::vector<Node_RAVE *>& route, int v){
    int route_len = route.size();
    std::vector<action::place> RAVE_action;
    if(route_len != 0)
        RAVE_action.push_back(route[route_len-1]->action_taken);
    int RAVE_action_len = RAVE_action.size();

    for(int i=route_len-2;i>=0;i--){
        //printf("current last_action = ");
        //std::cout << RAVE_action.back() << std::endl;
        //update move/RAVE value
        route[i]->value += v;
        route[i]->nb += 1;
        route[i]->RAVE_value += v;
        route[i]->RAVE_nb += 1;
        //printf("current i = %d\n",i);
        //update RAVE value
        for(int j=RAVE_action_len-3;j>=0;j-=2){
            //std::cout << RAVE_action[j] << std::endl;
            //see if any children of this node took the same actions
            int kids_len = route[i]->kids.size();
            for(int k=0;k<kids_len;k++){
                if(route[i]->kids[k]->action_taken == RAVE_action[j]){
                    //-v because it is the value of next state
                    //printf("update RAVE values\n");
                    route[i]->kids[k]->RAVE_value -= v;
                    route[i]->kids[k]->RAVE_nb += 1;
                }
            }
        }
        /*
        for(int j=RAVE_action_len-2;j>=0;j-=2){
            //std::cout << RAVE_action[j] << std::endl;;
            //see if any children of this node took the same actions
            int kids_len = route[i]->kids.size();
            for(int k=0;k<kids_len;k++){
                //switch the opponent's move
                
                if(route[i]->kids[k]->action_taken == RAVE_action[j]){
                    //-v because it is the value of next state
                    printf("update RAVE values\n");
                    route[i]->kids[k]->RAVE_value -= v;
                    route[i]->kids[k]->RAVE_nb += 1;
                }
            }
        }
        */
        //opponent's turn
        v = -v;
        //update RAVE_action_len
        RAVE_action.push_back(route[i]->action_taken);
        RAVE_action_len = RAVE_action.size();
    }
}

void Node_RAVE::new_kids(){
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
                kids.push_back(new Node_RAVE(after, next_who, move));
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
                kids.push_back(new Node_RAVE(after, next_who, move));
        }

    }
    
    //printf("size of kids : %d\n",kids.size());

    return;
}

