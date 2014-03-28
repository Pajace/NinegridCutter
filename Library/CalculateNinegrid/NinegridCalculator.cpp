#include "NinegridCalculator.h"
#include "NinegridCalculatorImpl.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

#ifdef WIN32
	#include <direct.h>
	#include <Windows.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

using namespace hTC::Image::Ninegrid;
using namespace std;

NinegridCalculator::NinegridCalculator() : _pimpl(new NinegridCalculatorImpl()) {}


NinegridCalculator::~NinegridCalculator(void){
	delete this->_pimpl;
}

// private method
bool NinegridCalculator::createFolder(string folderPath){
	int isCreated = -1;
#ifdef WIN32
	isCreated = _mkdir(folderPath.c_str());
#else
	isCreated = mkdir(folderPath.c_str(), S_IRWXO | S_IRWXG | S_IRWXU);
#endif

	return (isCreated==0);
}
// private method



bool NinegridCalculator::RunCalculaterAndOutpuInfo(){
	string outputFolder("output");

	bool isCreated = this->createFolder(outputFolder);
	if (isCreated==false){
		cerr << "Create folder failed. - " << outputFolder <<  endl;
		return false;
	}

	cout << "Scan PNG images...." << endl;
	cout << "  ===================================================== " << endl;
	vector<string> pngFileList = this->_pimpl->GetPngFileListAtFolder("*.png");
	cout << "  ===================================================== [Finish]" << endl;

	string outputTextFile = string(outputFolder);
//#ifdef WIN32
	outputTextFile.append(FOLDER_SEPARATOR);
	outputTextFile.append("PNGImageInformation.txt");
//#else
//	outputTextFile.append("/PNGImageInformation.txt");
//#endif
	ofstream outputText( outputTextFile.c_str(), ios::out);

	if (outputText.is_open() == false){
		std::cerr << "Can not open output" << FOLDER_SEPARATOR << "PNGImageInformation.txt" << endl;
		return false;
	} else{
		cout << "Create output file: output" << FOLDER_SEPARATOR << "PNGImageInformation.txt";
	}

	if (pngFileList.size() == 0){
		outputText << "No PNG file in this folder." << endl;
		outputText.close();
		return true;
	}

	
	for(size_t index=0; index < pngFileList.size(); index++ ){
		ImageSize imageSize = {0};
		BYTE* imageRawBuffer = NULL;
		NineGridInfo nineGridInfo = {0};
		ImageSize imageSizeWithout9GridInfo = {0};

		bool isSuccess = this->_pimpl->GetPngRawBuffer(pngFileList.at(index), &imageRawBuffer, imageSize);
		if (isSuccess==false) {
			outputText << "Cannot get the image raw buffer : " << pngFileList.at(index) << endl;
			continue;
		}

		this->_pimpl->CalculateNineGridInfo(imageRawBuffer, imageSize, nineGridInfo);
		this->_pimpl->CalculateImageSizeWithoutNineGridInfo(imageSize, imageSizeWithout9GridInfo);
		this->_pimpl->SaveImageWithout9GridInfo(this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".trim.png", imageRawBuffer, imageSize, imageSizeWithout9GridInfo);

		outputText << pngFileList.at(index) << endl;
		outputText << "--------------------------------------" << endl;
		outputText << "NineGrid=\"left,top,right,bottom\" => NineGrid=\"" << //NineGrid="left,top,right,bottom"
			nineGridInfo.Left << ", " <<
			nineGridInfo.Top << ", " <<
			nineGridInfo.Rright << ", " <<
			nineGridInfo.Bottom << "\"" << endl;
		outputText << "Width x Height (Without Nine grid info) : " << 
			imageSizeWithout9GridInfo.width << " x " <<
			imageSizeWithout9GridInfo.heigh << " " << endl;
		outputText << "--------------------------------------" << endl << endl;

	}
	
	return true;
}

bool NinegridCalculator::RunSplitImageWith3H(){
	string outputFolder("output-3H");

	bool isCreated = createFolder(outputFolder);
	if (isCreated==false){
		cerr << "Create folder - [" << outputFolder << "] failed." << endl;
		return false;
	}

	vector<string> pngFileList = this->_pimpl->GetPngFileListAtFolder("*.png");
	string outputFolderFile(outputFolder.c_str());
	outputFolderFile.append(FOLDER_SEPARATOR);
	outputFolderFile.append("Split3HImage.log");

	ofstream outputText( outputFolderFile.c_str(), ios::out);
	if (outputText.is_open() == false){		
		return false;
	}

	if (pngFileList.size() == 0){
		outputText << "No PNG file in this folder." << endl;
		outputText.close();
		return true;
	}


	for(size_t index=0; index < pngFileList.size(); index++ ){
		ImageSize imageSize = {0};
		BYTE* imageRawBuffer = NULL;
		BYTE* imageRawBufferWithout9GridInfo = NULL;
		NineGridInfo nineGridInfo = {0};
		ImageSize imageSizeWithout9GridInfo = {0};

		bool isSuccess = this->_pimpl->GetPngRawBuffer(pngFileList.at(index), &imageRawBuffer, imageSize);
		if (isSuccess==false) {
			outputText << "Cannot get the image raw buffer : " << pngFileList.at(index) << endl;
			continue;
		}

		this->_pimpl->CalculateNineGridInfo(imageRawBuffer, imageSize, nineGridInfo);
		if (nineGridInfo.Top <= 0 || nineGridInfo.Bottom <= 0) {
			delete [] imageRawBuffer;
			outputText << "File Name: " << pngFileList.at(index) << endl;
			outputText << " => Nine grid's number is inavlid, any of nine grid top or bootm can not be zero." << endl;
			outputText << "    [Top: " << nineGridInfo.Top << ", Bottom: " << nineGridInfo.Bottom << "]" << endl;
			outputText << " => split horizontal image fail." << endl;
		}

		this->_pimpl->CalculateImageSizeWithoutNineGridInfo(imageSize, imageSizeWithout9GridInfo);
		vector<size_t> bufferSizeIn3H = this->_pimpl->GetSplitRawBufferSizeByHorizontal(imageSizeWithout9GridInfo, nineGridInfo);
		this->_pimpl->Trim9GridInfo(imageRawBuffer, imageSize, imageSizeWithout9GridInfo, &imageRawBufferWithout9GridInfo);

		vector<BYTE*> bufferIn3H = this->_pimpl->GetSplitRawBufferByHorizontal(imageRawBufferWithout9GridInfo, imageSizeWithout9GridInfo, nineGridInfo);


		ImageSize splitImageSize = {0};
		
		splitImageSize.width = imageSizeWithout9GridInfo.width;
		splitImageSize.heigh = nineGridInfo.Top;
		splitImageSize.dataSize = bufferSizeIn3H.at(0);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".top.png", bufferIn3H.at(0), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;
		
		splitImageSize.width = imageSizeWithout9GridInfo.width;
		splitImageSize.heigh = imageSizeWithout9GridInfo.heigh - nineGridInfo.Top - nineGridInfo.Bottom;
		splitImageSize.dataSize = bufferSizeIn3H.at(1);

		string fileName = this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".middle.png";
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( fileName, bufferIn3H.at(1), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;
		
		splitImageSize.width = imageSizeWithout9GridInfo.width;
		splitImageSize.heigh = nineGridInfo.Bottom;
		splitImageSize.dataSize = bufferSizeIn3H.at(2);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".bottom.png", bufferIn3H.at(2), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;
		

		for (size_t i = 0 ; i<bufferIn3H.size(); i++)
			delete [] bufferIn3H.at(i);
		delete [] imageRawBuffer;
	}
	
	return true;
}

bool NinegridCalculator::RunSplitImageWith3V(){
	string outputFolder("output-3V");
	
	bool isCreated = createFolder(outputFolder);
	if (isCreated==false){
		cerr << "Create folder - [" << outputFolder << "] failed." << endl;
		return false;
	}

	vector<string> pngFileList = this->_pimpl->GetPngFileListAtFolder("*.png");
	string outputFolderFile(outputFolder.c_str());
	outputFolderFile.append(FOLDER_SEPARATOR);
	outputFolderFile.append("Split3VImage.log");

	ofstream outputText( outputFolderFile.c_str(), ios::out);
	if (outputText.is_open() == false){		
		return false;
	}

	if (pngFileList.size() == 0){
		outputText << "No PNG file in this folder." << endl;
		outputText.close();
		return true;
	}


	for(size_t index=0; index < pngFileList.size(); index++ ){
		ImageSize imageSize = {0};
		BYTE* imageRawBuffer = NULL;
		BYTE* imageRawBufferWithout9GridInfo = NULL;
		NineGridInfo nineGridInfo = {0};
		ImageSize imageSizeWithout9GridInfo = {0};

		bool isSuccess = this->_pimpl->GetPngRawBuffer(pngFileList.at(index), &imageRawBuffer, imageSize);
		if (isSuccess==false) {
			outputText << "Cannot get the image raw buffer : " << pngFileList.at(index) << endl;
			continue;
		}

		this->_pimpl->CalculateNineGridInfo(imageRawBuffer, imageSize, nineGridInfo);
		if (nineGridInfo.Left == 0 || nineGridInfo.Rright ==0 || nineGridInfo.Top ==0 || nineGridInfo.Bottom ==0) {
			outputText << "File Name: " << pngFileList.at(index) << endl;
			outputText << " => Nine grid's number is invalid. Any of nine grid L or R can not be zero." <<
				"L: " << nineGridInfo.Left << ", R: " << nineGridInfo.Rright << endl;
			outputText << " => Split vertical image failed." << endl;
			delete [] imageRawBuffer;
			continue;
		}

		this->_pimpl->CalculateImageSizeWithoutNineGridInfo(imageSize, imageSizeWithout9GridInfo);
		vector<size_t> bufferSizeIn3V = this->_pimpl->GetSplitRawBufferSizeByVertical(imageSizeWithout9GridInfo, nineGridInfo);
		this->_pimpl->Trim9GridInfo(imageRawBuffer, imageSize, imageSizeWithout9GridInfo, &imageRawBufferWithout9GridInfo);

		vector<BYTE*> bufferIn3V = this->_pimpl->GetSplitRawBufferByVertical(imageRawBufferWithout9GridInfo, imageSizeWithout9GridInfo, nineGridInfo);


		ImageSize splitImageSize = {0};

		splitImageSize.width = nineGridInfo.Left;
		splitImageSize.heigh = imageSizeWithout9GridInfo.heigh;
		splitImageSize.dataSize = bufferSizeIn3V.at(0);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".left.png", bufferIn3V.at(0), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = imageSizeWithout9GridInfo.width - nineGridInfo.Left - nineGridInfo.Rright;
		splitImageSize.heigh = imageSizeWithout9GridInfo.heigh;
		splitImageSize.dataSize = bufferSizeIn3V.at(1);

		string fileName = this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".middle.png";
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( fileName, bufferIn3V.at(1), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = nineGridInfo.Rright;
		splitImageSize.heigh = imageSizeWithout9GridInfo.heigh;
		splitImageSize.dataSize = bufferSizeIn3V.at(2);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".right.png", bufferIn3V.at(2), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;


		for (size_t i = 0 ; i<bufferIn3V.size(); i++)
			delete [] bufferIn3V.at(i);
		delete [] imageRawBuffer;
	}

	return true;
}

bool NinegridCalculator::RunSplitImageWith9Grid(){
	string outputFolder("output-9Grid");
	
	bool isCreated = createFolder(outputFolder);
	if (isCreated==false){
		cerr << "Create folder - [" << outputFolder << "] failed." << endl;
		return false;
	}

	vector<string> pngFileList = this->_pimpl->GetPngFileListAtFolder("*.png");

	string outputFolderFile(outputFolder.c_str());
	outputFolderFile.append(FOLDER_SEPARATOR);
	outputFolderFile.append("Split9GridImage.log");
	ofstream outputText( outputFolderFile.c_str(), ios::out);
	if (outputText.is_open() == false){		
		return false;
	}

	if (pngFileList.size() == 0){
		outputText << "No PNG file in this folder." << endl;
		outputText.close();
		return true;
	}


	for(size_t index=0; index < pngFileList.size(); index++ ){
		ImageSize imageSize = {0};
		BYTE* imageRawBuffer = NULL;
		BYTE* imageRawBufferWithout9GridInfo = NULL;
		NineGridInfo nineGridInfo = {0};
		ImageSize imageSizeWithout9GridInfo = {0};

		bool isSuccess = this->_pimpl->GetPngRawBuffer(pngFileList.at(index), &imageRawBuffer, imageSize);
		if (isSuccess==false) {
			outputText << "Cannot get the image raw buffer : " << pngFileList.at(index) << endl;
			continue;
		}

		this->_pimpl->CalculateNineGridInfo(imageRawBuffer, imageSize, nineGridInfo);

		if (nineGridInfo.Bottom==0 || nineGridInfo.Top ==0 || nineGridInfo.Left ==0 || nineGridInfo.Rright ==0) {
			outputText << "File name: " << pngFileList.at(index) << endl;
			outputText << "  => ninegrid format invalid." << endl;
			outputText << "  => any of nine grid number cannot be zero [" << 
				"L: " << nineGridInfo.Left << ", R: " << nineGridInfo.Rright <<
				", T: " << nineGridInfo.Top  << ", B: " << nineGridInfo.Bottom << "]" << endl;
			outputText << "  => split nine grid image fail" << endl;

			delete [] imageRawBuffer;
			continue;
		}


		this->_pimpl->CalculateImageSizeWithoutNineGridInfo(imageSize, imageSizeWithout9GridInfo);
		vector<size_t> bufferSizeIn9Grid = this->_pimpl->GetSplitRawBufferSizeBy9Grid(imageSizeWithout9GridInfo, nineGridInfo);
		this->_pimpl->Trim9GridInfo(imageRawBuffer, imageSize, imageSizeWithout9GridInfo, &imageRawBufferWithout9GridInfo);
		vector<BYTE*> bufferIn9Grid = this->_pimpl->GetSplitRawBufferBy9Grid(imageRawBufferWithout9GridInfo, imageSizeWithout9GridInfo, nineGridInfo);


		size_t top_height = nineGridInfo.Top;
		size_t mid_height = imageSizeWithout9GridInfo.heigh - nineGridInfo.Top - nineGridInfo.Bottom;
		size_t bottom_height = nineGridInfo.Bottom;
		size_t left_width = nineGridInfo.Left;
		size_t mid_width = imageSizeWithout9GridInfo.width - nineGridInfo.Left - nineGridInfo.Rright;
		size_t right_width = nineGridInfo.Rright;

		ImageSize splitImageSize = {0};

		splitImageSize.width = left_width;
		splitImageSize.heigh = top_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(0);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".left-top.png", bufferIn9Grid.at(0), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = mid_width;
		splitImageSize.heigh = top_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(1);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".middle-top.png", bufferIn9Grid.at(1), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = right_width;
		splitImageSize.heigh = top_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(2);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".right-top.png", bufferIn9Grid.at(2), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = left_width;
		splitImageSize.heigh = mid_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(3);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".middle-left.png", bufferIn9Grid.at(3), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = mid_width;
		splitImageSize.heigh = mid_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(4);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".middle-middle.png", bufferIn9Grid.at(4), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = right_width;
		splitImageSize.heigh = mid_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(5);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".middle-right.png", bufferIn9Grid.at(5), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = left_width;
		splitImageSize.heigh = bottom_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(6);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".bottom-left.png", bufferIn9Grid.at(6), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = mid_width;
		splitImageSize.heigh = bottom_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(7);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".bottom-middle.png", bufferIn9Grid.at(7), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;

		splitImageSize.width = right_width;
		splitImageSize.heigh = bottom_height;
		splitImageSize.dataSize = bufferSizeIn9Grid.at(8);
		isSuccess = this->_pimpl->SaveRawBufToPngFormat( this->_pimpl->GetBaseFileName( outputFolder + FOLDER_SEPARATOR + pngFileList.at(index))+".bottom-right.png", bufferIn9Grid.at(8), splitImageSize );
		if (!isSuccess) outputText << "Save PNG raw buffer to PNG format failed." << endl;


		for (size_t i = 0 ; i<bufferIn9Grid.size(); i++)
			delete [] bufferIn9Grid.at(i);
		delete [] imageRawBuffer;
	}

	return true;
}