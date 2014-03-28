#include <NinegridCalculator.h>
#include <string>
#include <iostream>

using namespace std;
using namespace hTC::Image::Ninegrid;

int main(){

	NinegridCalculator *c = new NinegridCalculator();
	cout << "Running...., please wait.....";

	bool isSuccess = c->RunCalculaterAndOutpuInfo();
	if (isSuccess) {
		cout << "[Finish]" << endl;
	} else {
		cerr << "[Fail]" << endl;
	}

	isSuccess = c->RunSplitImageWith3H();
	if (isSuccess) {
		cout << "[Finish]" << endl;
	} else {
		cerr << "[Fail]" << endl;
	}

	isSuccess = c->RunSplitImageWith3V();
	if (isSuccess) {
		cout << "[Finish]" << endl;
	} else {
		cerr << "[Fail]" << endl;
	}

	isSuccess = c->RunSplitImageWith9Grid();
	if (isSuccess) {
		cout << "[Finish]" << endl;
	} else {
		cerr << "[Fail]" << endl;
	}

	system("Pause");
	delete c;	
	return 0;
}