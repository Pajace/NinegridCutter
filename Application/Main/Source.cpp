#include <NinegridCalculator.h>
#include <string>
#include <iostream>

#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using namespace std;
using namespace hTC::Image::Ninegrid;

string GetCurrentFolderPath() {
	char cCurrentPath[FILENAME_MAX];

	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		cerr << "Get current path failed." << endl;

	string result(cCurrentPath);
	return result;
}

void printSuccessOrFailMessage(bool isSuccess) {
	if (isSuccess) {
		cout << " [Success]" << endl;
	} else {
		cerr << " [Fail]" << endl;
	}
}

int main(int argc, char* argv[]){

#ifndef WIN32
	string execFileName(argv[0]);
	int pos = execFileName.rfind("/");
	string currentPath = execFileName.substr(0, pos);

	cout << "Switch dir to => " << currentPath;
	if( chdir(currentPath.c_str()) != 0)
		cout << " [FAIL]" << endl;
	else
		cout << " [SUCCESS]" << endl;
#endif

	NinegridCalculator *c = new NinegridCalculator();
	cout << "Running...." << endl;

	cout << endl;
	cout << "1) trim image: ";
	bool isSuccess = c->RunCalculaterAndOutpuInfo();
	printSuccessOrFailMessage(isSuccess);

	cout << "2) Split images - vertical part: ";
	isSuccess = c->RunSplitImageWith3H();
	printSuccessOrFailMessage(isSuccess);

	cout << "3) Split images - horizontal: ";
	isSuccess = c->RunSplitImageWith3V();
	printSuccessOrFailMessage(isSuccess);

	cout << "4) Split images - 9 part: ";
	isSuccess = c->RunSplitImageWith9Grid();
	printSuccessOrFailMessage(isSuccess);

	system("Pause");
	delete c;	
	return 0;

}