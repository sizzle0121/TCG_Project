#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"

#define gamma 1

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss(args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			property[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string name() const {
		auto it = property.find("name");
		return it != property.end() ? std::string(it->second) : "unknown";
	}
protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> property;
};

/**
 * evil (environment agent)
 * add a new random tile on board, or do nothing if the board is full
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public agent {
public:
	rndenv(const std::string& args = "") : agent("name=rndenv " + args) {
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
	}

	virtual action take_action(const board& after) {
		int space[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		std::shuffle(space, space + 16, engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;
			std::uniform_int_distribution<int> popup(0, 9);
			int tile = popup(engine) ? 1 : 2;
			return action::place(tile, pos);
		}
		return action();
	}

private:
	std::default_random_engine engine;
};

/**
 * TODO: player (non-implement)
 * always return an illegal action
 */
class player : public agent {
public:
	player(const std::string& args = "") : agent("name=player " + args), alpha(0.0025f) {
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
		if (property.find("alpha") != property.end())
			alpha = float(property["alpha"]);

		if (property.find("load") != property.end())
			load_weights(property["load"]);
		//initialize the n-tuple network
		else{
			if(weights.size() == 0){
				for(int i=0; i<8; i++){
					weights.push_back(weight(24*24*24*24));
				}
			}
		}
	}
	~player() {
		if (property.find("save") != property.end())
			save_weights(property["save"]);
	}

	virtual void open_episode(const std::string& flag = "") {
		episode.clear();
	}

	virtual void close_episode(const std::string& flag = "") {
		//train the n-tuple network by TD(0)
		int power[4] = {1, 24, 24*24, 24*24*24 };
		bool first = true;
		while(episode.size() > 0){
			board a = episode.back().after, b = episode.back().before;
			int R = episode.back().reward;
			float S_ = getStateValue(a), S = getStateValue(b);
			for(int j=0; j<8; j++){//total 8 4-tuple
				size_t match_weight_b = 0;
				//calculate the idx of the weight for each tuple
				if(j < 4){
					for(int k=0; k<4; k++)
						match_weight_b += (power[k] * b(j*4 + k));
				}else if(j < 8){
					for(int k=0; k<4; k++)
						match_weight_b += (power[k] * b(j%4 + k*4));
				}
				//TD(0)
				if(!first){
					float data = weights[j][match_weight_b] + (alpha*(R + gamma*S_ - S));
					weights[j].access(match_weight_b, data);
				}
				else{
					float data = weights[j][match_weight_b] + (alpha*(0 - S));
					weights[j].access(match_weight_b, data);
				}
			}
			episode.pop_back();
			first = false;
		}
	}

	float getStateValue(board& b){
		float sum = 0.0;
		int power[4] = {1, 24, 24*24, 24*24*24 };
		for(int i=0; i<8; i++){
			size_t tmp = 0;
			if(i < 4){
				for(int j=0; j<4; j++)
					tmp += (power[j] * b(i*4 + j));
				sum += weights[i][tmp];
			}else if(i < 8){
				for(int j=0; j<4; j++)
					tmp += (power[j] * b(i%4 + j*4));
				sum += weights[i][tmp];
			}
		}
		return sum;
	}

	virtual action take_action(const board& before) {
		//select a proper action
		float max_value = -1111111;
		int max_op = -1;
		float current_move_value;
		bool rec = false;
		for(int op=0; op<4; op++){
			board b = before;
			int current_move = b.move(op);
			current_move_value = getStateValue(b);
			if(current_move_value > 0)
				rec = true;
			if(current_move != -1){
				if(current_move + current_move_value >= max_value){
					max_value = current_move + current_move_value;
					max_op = op;
				}
			}
		}
		//random action
		int opcode[] = { 0, 1, 2, 3 };
		if(!rec){
			std::shuffle(opcode, opcode + 4, engine);
			for (int op : opcode) {
				board b = before;
				if (b.move(op) != -1) max_op = op;
			}
		}
		//push the step into episode
		board b = before;

		state tmp;
		if(episode.size() > 0)
			episode.back().after = b;
		tmp.before = b;
		if(max_op != -1)	tmp.reward = b.move(max_op);

		episode.push_back(tmp);
		if(max_op != -1)	return action::move(max_op);
		else				return action();
	}

public:
	virtual void load_weights(const std::string& path) {
		std::ifstream in;
		in.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		weights.resize(size);
		for (weight& w : weights)
			in >> w;
		in.close();
	}

	virtual void save_weights(const std::string& path) {
		std::ofstream out;
		out.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		size_t size = weights.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : weights)
			out << w;
		out.flush();
		out.close();
	}

private:
	std::vector<weight> weights;

	struct state {
		board before;
		board after;
		action move;
		int reward;
	};

	std::vector<state> episode;
	float alpha;

private:
	std::default_random_engine engine;
};
