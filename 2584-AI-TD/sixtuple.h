#ifndef __SIXTUPLE_H__
#define __SIXTUPLE_H__

#include <iostream>
#include <cstdlib>
#include "board.h"

extern std::vector<weight> weights;

bool q = false;

void construct_SixTuple(){
	weights.push_back(weight(24*24*24*24*24*24));//0
	weights.push_back(weight(24*24*24*24*24*24));//1
	weights.push_back(weight(24*24*24*24));//2
	weights.push_back(weight(24*24*24*24));//3
}
unsigned int accessSixTuple(int num, board& b){//num = 0, 1
	unsigned int idx = 0;
	unsigned int power[6] = {1, 24, 24*24, 24*24*24, 24*24*24*24, 24*24*24*24*24};
	int cnt = 0;
	for(int i = num; i < num+2; ++i){
		for(int j = 0; j < 3; ++j){
			idx += (power[cnt++] * b(i + j*4));
		}
	}
	return idx;
}
unsigned int accessFourTuple(int num, board& b){//num = 2, 3
	unsigned int idx = 0;
	unsigned int power[4] = {1, 24, 24*24, 24*24*24};
	for(int i = 0; i < 4; ++i){
		idx += (power[i] * b(num + i*4));
	}
	return idx;
}
float getStateValue(board& b){
	if(q){
		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				std::cout << b(i*4 + j) << "  ";
			}std::cout << std::endl;
		}
	}


	float sum = 0.0;
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
		cnt++;
		//6-tuple
		for(int i = 0; i < 2; ++i){
			unsigned int tmp = accessSixTuple(i, tmpb);
			sum += weights[i][tmp];
		}
		//4-tuple
		for(int i = 2; i < 4; ++i){
			unsigned int tmp = accessFourTuple(i, tmpb);
			sum += weights[i][tmp];
		}
	}
	return sum;
}

#endif