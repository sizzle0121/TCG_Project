#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <cstdlib>
#include "board.h"
#include "action.h"
#include "weight.h"
#include "sixtuple.h"
#include "expectimax.h"

#define gamma 1
std::vector<weight> weights;


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
			std::uniform_int_distribution<int> popup(1, 100);
			int tile = (popup(engine) <= 90) ? 1 : 3;
			return action::place(tile, pos);
		}
		return action();
	}

private:
	std::default_random_engine engine;
};


class player : public agent {
public:
	player(const std::string& args = "") : agent("name=player " + args), alpha(0.0025f) {//025f
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
		if (property.find("alpha") != property.end())
			alpha = float(property["alpha"]);

		if (property.find("load") != property.end())
			load_weights(property["load"]);
		else{
			if(weights.size() == 0){
				construct_SixTuple();
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
		bool first = true;
		while(episode.size() > 0){
			board a = episode.back().after, b = episode.back().before;
			int R = episode.back().reward;
			float S_ = getStateValue(a), S = getStateValue(b);
			int T = 8, cnt = 0;
			while(T--){
				board tmpb = b;
				switch(cnt){
					case 0:
						break;
					case 1:
						tmpb.reflect_horizontal();
						break;
					case 2:
						tmpb.reflect_vertical();
						break;
					case 3:	
						tmpb.reflect_horizontal();
						tmpb.reflect_vertical();
						break;
					case 4:
						tmpb.rotate_right();
						break;
					case 5:
						tmpb.rotate_right();
						tmpb.reflect_horizontal();
						break;
					case 6:
						tmpb.rotate_right();
						tmpb.reflect_vertical();
						break;
					case 7:
						tmpb.rotate_right();
						tmpb.reflect_horizontal();
						tmpb.reflect_vertical();
						break;
				}
				for(int i = 0; i < 4; ++i){
					unsigned int match_weight_b = (i < 2)? accessSixTuple(i, tmpb) : accessFourTuple(i, tmpb);
					//TD(0)
					if(!first){
						float data = weights[i][match_weight_b] + (alpha*(R + S_ - S));
						weights[i].access(match_weight_b, data);
					}
					else{
						float data = weights[i][match_weight_b] + (alpha*(0 - S));
						weights[i].access(match_weight_b, data);
					}
				}
				cnt++;
			}
			episode.pop_back();
			first = false;
		}
	}

	virtual action take_action(const board& before) {
		int max_op = -1;
		Expectimax find_act(3);
		max_op = find_act.searchACT(before);
		board b = before;
		state tmp;
		if(max_op != -1){
			tmp.reward = find_act.getExpectValue();
			b.move(max_op);
			tmp.before = b;
			if(episode.size() > 0)	episode.back().after = b;
			episode.push_back(tmp);
			return action::move(max_op);
		}
		else
			return action();
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
