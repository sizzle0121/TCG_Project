#pragma once
#include <array>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <queue>
/**
 * array-based board for 2048
 *
 * index (2-d form):
 * [0][0] [0][1] [0][2] [0][3]
 * [1][0] [1][1] [1][2] [1][3]
 * [2][0] [2][1] [2][2] [2][3]
 * [3][0] [3][1] [3][2] [3][3]
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */

int fib[25];
void construct_fib(){
	fib[0] = 1;
	fib[1] = 1;
	for(int i=2; i<25; i++){
		fib[i] = fib[i-1] + fib[i-2];
	}
	fib[0] = 0;
}


class board {

public:
	board() : tile(), empty_blk(0){}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	std::array<int, 4>& operator [](const int& i) { return tile[i]; }
	const std::array<int, 4>& operator [](const int& i) const { return tile[i]; }
	int& operator ()(const int& i) { return tile[i / 4][i % 4]; }
	const int& operator ()(const int& i) const { return tile[i / 4][i % 4]; }



public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:
	/**
	 * apply an action to the board
	 * return the reward gained by the action, or -1 if the action is illegal
	 */


	int move(const int& opcode) {
		switch (opcode) {
		case 0: return move_up();
		case 1: return move_right();
		case 2: return move_down();
		case 3: return move_left();
		default: return -1;
		}
	}

	int move_left() {
		board prev = *this;
		int score = 0;
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];
			int top = 0, hold = 0;
			for (int c = 0; c < 4; c++) {
				int tile = row[c];
				if (tile == 0) continue;
				row[c] = 0;
				if (hold) {
					if (abs(tile-hold) == 1 || (tile == hold && tile == 1)) {//merge 
						row[top++] = std::max(tile, hold) + 1;//max(tile, hold) + 1
						score += fib[std::max(tile, hold) + 1];	//modify tile
						hold = 0;
					} else {//no merge
						row[top++] = hold;
						hold = tile;
					}
				} else {
					hold = tile;
				}
			}
			if (hold) tile[r][top] = hold;
		}

		max_tile = 0;
		max_tile_i = 0;
		max_tile_j = 0;
		empty_blk = 0;
		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				if(tile[i][j] == 0)
					empty_blk++;
				if(tile[i][j] > max_tile){
					max_tile = tile[i][j];
					max_tile_i = i;
					max_tile_j = j;
				}
					
			}
		}

		return (*this != prev) ? score : -1;
	}
	int move_right() {
		reflect_horizontal();
		int score = move_left();
		reflect_horizontal();
		return score;
		return score;
	}
	int move_up() {
		rotate_right();
		int score = move_right();
		rotate_left();
		return score;
	}
	int move_down() {
		rotate_right();
		int score = move_left();
		rotate_left();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(const int& r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

	int getEmptyBlk(){
		return empty_blk;
	}

	int getMonoDec(){
		bool used[4][4];
		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				used[i][j] = false;
			}
		}
		int accum = 0;
		int walk[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};
		std::queue<int> qi, qj;
		qi.push(max_tile_i);
		qj.push(max_tile_j);
		used[max_tile_i][max_tile_j] = true;
		while(qi.size() != 0){
			int x = qi.front(), y = qj.front();
			qi.pop();
			qj.pop();
			for(int i=0; i<4; i++){
				int tx = x + walk[i][0], ty = y + walk[i][1];
				if(tx < 4 && tx >= 0 && ty < 4 && ty >= 0 && used[tx][ty] == false){
					if(tile[x][y] < tile[tx][ty]){
						accum += tile[tx][ty] - tile[x][y];
					}
					qi.push(tx);
					qj.push(ty);
					used[tx][ty] = true;
				}
			}
		}
		return accum;
	}

public:
    friend std::ostream& operator <<(std::ostream& out, const board& b) {
		char buff[32];
		out << "+------------------------+" << std::endl;
		for (int r = 0; r < 4; r++) {
			std::snprintf(buff, sizeof(buff), "|%6u%6u%6u%6u|",
				fib[b[r][0]],
				fib[b[r][1]],
				fib[b[r][2]],
				fib[b[r][3]]);
			out << buff << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

private:
    std::array<std::array<int, 4>, 4> tile;
    int empty_blk;
    int max_tile, max_tile_i, max_tile_j;
};
