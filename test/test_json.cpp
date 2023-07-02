#include <json/json.h>
#include <iostream>
#define _ auto
int main() {
	_ x = 4;
	Json::Value root;
	root["32435"][0] = 6;
	std::cout << root.toStyledString();
	return 0;
}