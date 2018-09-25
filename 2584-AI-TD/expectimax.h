#include <iostream>
#include <cstdlib>
#include "sixtuple.h"

#define ROW 4
#define COLUMN 4
#define PAUSE printf("Press Enter key to continue..."); fgetc(stdin);

int tt = 0;

class Expectimax{
public:
	Expectimax(int d): _depth(d), expect_value(0.0){}
	int searchACT(board b){
		int best_ACT = -1;//initialize the given board to dead
		dfs_ai(b, _depth, best_ACT);
		return best_ACT;
	}

	float getExpectValue(){
		return expect_value;
	}


private:
	float dfs_ai(board b, int depth, int& best_ACT){
		float maxi = -1e20;
		for(int op = 0; op < 4; ++op){
			board tmp = b;
			int reward = tmp.move(op);
			if(reward != -1){
				float r = (depth-1 < 0)? 0 : dfs_env(tmp, depth-1);
				float state_value = getStateValue(tmp);
				if(depth == _depth){//layer 0
					if(r + reward + state_value > maxi){
						maxi = r + reward + state_value;
						best_ACT = op;
						expect_value = reward;
					}
				}else if(r + reward + state_value > maxi){
					maxi = r + reward + state_value;
				}

			}
		}
		if(maxi > -1e20)	return maxi;
		else	return 0;
	}

	float dfs_env(board b, int depth){
		float mean = 0.0;
		int cnt = 0;
		for(int k = 1; k <= 3; k += 2){
			for(int i = 0; i < ROW; ++i){
				for(int j = 0; j < COLUMN; ++j){
					board tmp = b;
					if(tmp(i*4 + j) == 0){
						tmp(i*4 + j) = k;
						int fake_best_ACT;
						float r = (depth-1 < 0)? 0 : dfs_ai(tmp, depth-1, fake_best_ACT);
						if(k == 1){
							r *= 0.9;
							cnt++;
						}else{
							r *= 0.1;
						}
						mean += r;
					}
				}
			}
		}
		if(cnt != 0)	mean /= cnt;
		return mean;
	}

private:
	int _depth;
	float expect_value;
};