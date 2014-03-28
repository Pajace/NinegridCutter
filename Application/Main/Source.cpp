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

int main(int argc, char* argv[]){

#ifdef WIN32
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
#else
//cout << "The current path is => " << GetCurrentFolderPath() << endl;

	string execFileName(argv[0]);
	int pos = execFileName.rfind("/");
	string currentPath = execFileName.substr(0, pos);

	cout << "Switch dir to => " << currentPath;
	if( chdir(currentPath.c_str()) != 0)
		cout << " [FAIL]" << endl;
	else
		cout << " [SUCCESS]" << endl;

	NinegridCalculator *c = new NinegridCalculator();

	bool isSuccess = c->RunCalculaterAndOutpuInfo();
	if (isSuccess) {
		cout << endl <<  "Convert Finish!" << endl;
	} else {
		cerr << endl << "Convert Fail." << endl;
	}
#endif
}