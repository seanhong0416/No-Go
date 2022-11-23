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

class MCTS : public random_agent{
public:
	MCTS(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
	}

	virtual action take_action(const board& state) {
		std::shuffle(space.begin(), space.end(), engine);
		for (const action::place& move : space) {
			board after = state;
			if (move.apply(after) == board::legal)
				return move;
		}
		return action();
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
};

class Node{
public:
    Node(board b_in, board::piece_type who_in):
    nb(0), space(board::size_x * board::size_y)
    {
        b = b_in;
        who = who_in;

        for(size_t i=0;i<space.size();i++){
            space[i] = action::place(i, who);
        }
        //get all legal_moves
        for (const action::place& move : space) {
			board after = b_in;
			if (move.apply(after) == board::legal)
				legal_moves.push_back(move);
		}
    }

    void new_kid(board b_in, board::piece_type who_in);

private:
    std::vector<action::place> space;       //init
    board::piece_type who;                  //init

    std::vector<action::place> legal_moves; //init
    std::vector<Node> kids;
    board b;                                //init
    int nb;                                 //init
};