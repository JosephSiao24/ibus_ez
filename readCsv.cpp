#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

int main(void){
	std::ifstream file("/home/joseph/Desktop/HelloWorldProject/outCsv.csv");
	if(!file.is_open()){
		std::cerr << "无法打开文件" << std::endl;
		return 1;
	}
	string line;
	while(getline(file,line)){
		
		stringstream ss(line);
		string field;
		while(getline(ss, field)){
			cout << field << "s" << endl;
		}
	}
	file.close();
}
