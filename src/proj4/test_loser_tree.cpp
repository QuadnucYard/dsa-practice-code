#include "ds/loser_tree.hpp"
#include <iostream>
#include <vector>

int main() {
	std::vector<int> input_data{51, 49, 39, 46, 38, 29, 14, 61, 15, 30, 1,	48,
								52, 3,	63, 27, 4,	13, 89, 24, 46, 58, 33, 76};
	loser_tree<std::pair<int, int>> lt(32);
	size_t I = 0;
	for (int i = lt.size() - 1; i >= 0; i--) {
		if (lt.size() - 1 - i >= input_data.size()) {
			lt.push_at({2, 0}, i);
			continue;
			;
		}
		lt.push_at({1, input_data[I++]}, i);
	}
	int rc = 1, rmax = 1;
	while (rc <= rmax) {
		printf("%d: ", rc);
		int cnt = 0;
		while (lt.top().first == rc) {
			int minimax = lt.top().second;
			cnt++;
			printf("%d ", minimax);
			// 如果输入文件结束，则虚设一条记录（属＂rmax+1＂段）
			if (I >= input_data.size()) {
				lt.push({rmax + 1, 0});
			} else { // 输入文件非空时
				// 从输入文件读入下一记录
				int x = input_data[I++];
				if (x < minimax) {
					// 新读入的记录比上一轮的最小关键字还小，则它属下一段
					rmax = rc + 1;
					lt.push({rmax, x});
				} else {
					// 新读入的记录大则属当前段
					lt.push({rc, x});
				}
			}
		}
		printf("   [%d]\n", cnt);
		rc = lt.top().first;
	}
	return 0;
}