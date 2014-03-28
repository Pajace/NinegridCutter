#include "NinegridCalculatorImpl.h"
#include "PNGCodec.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/types.h>
	#include <dirent.h>
	#include <unistd.h>
	#define getCurrentDir getcwd
#endif
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <new>
#include <assert.h>

using namespace hTC::Image::Ninegrid;
using namespace std;
using namespace hTC::ComposerLib;

NinegridCalculatorImpl::NinegridCalculatorImpl(){
}


NinegridCalculatorImpl::~NinegridCalculatorImpl(void){}

//  __out BYTE **imgRawBuf, __out ImageSize &pngRawBufImgSize
bool NinegridCalculatorImpl::GetPngRawBuffer(const std::string& fileName, BYTE **imgRawBuf, ImageSize &pngRawBufImgSize){
	PNGBuffer destBuffer;
	PNGCodec  pngcodec;
	unsigned long width = 0;
	unsigned long height = 0;


	PNGCodec::ENCODE_ERROR_NO errNo = pngcodec.readPNGFromPath(destBuffer, width, height, fileName.c_str());
	if (errNo != PNGCodec::ENCODE_OK) {
		cerr << "[GetPngRawBuffer][Fatal] Decode PNG file failed."<< endl;
		return false;
	}

	if (destBuffer.size()<=0) {
		cerr <<"[GetPngRawBuffer][Fatal] The destBuffer is 0 after decode png."<< endl;
		return false;
	}

	if (imgRawBuf==NULL) {
		cerr <<"[GetPngRawBuffer][Fatal] out parameter imgRawBuf CANNOT BE NULL"<< endl;
		return false;
	}
	*imgRawBuf = (BYTE*)allocMemory(destBuffer.size());
	if (*imgRawBuf==NULL) {
		cerr <<"[GetPngRawBuffer][Fatal](%d) Allocate memory failed." << endl;
		destBuffer.unwrap();
		return false;
	}

#ifdef WIN32
	errno_t memCpyResult = memcpy_s(*imgRawBuf, destBuffer.size(), destBuffer.toPNGBytes(), destBuffer.size());
	if (memCpyResult!=0) {
		cerr <<"[GetPngRawBuffer](%d) memcpy_s failed." << endl;
		destBuffer.unwrap();
		return false;
	}
#else
	memcpy(*imgRawBuf, destBuffer.toPNGBytes(), destBuffer.size());
#endif

	pngRawBufImgSize.width = width;
	pngRawBufImgSize.heigh = height;
	pngRawBufImgSize.dataSize = destBuffer.size();
	destBuffer.unwrap();

	return true;
}
// __out NineGridInfo &nineGridInfo
bool NinegridCalculatorImpl::CalculateNineGridInfo(BYTE *imgRawBuf, ImageSize &imgRawBufSize, NineGridInfo &nineGridInfo){
	const char NINEGRID_BLACK_LINE[] = { 0x00, 0x00, 0x00, 0xff};
	const size_t BGRA_BYTE = 4;

	size_t blackLineStart = 0;
	size_t blackLineStop = 0;
	size_t currentPosition = 0;
	bool blackLineMode = false;

	for(size_t i=0; i<imgRawBufSize.width*BGRA_BYTE; i+=BGRA_BYTE){
		bool isBlackLine = (memcmp(imgRawBuf+i, NINEGRID_BLACK_LINE, BGRA_BYTE)==0);

		if (blackLineMode){
			if (isBlackLine){
				currentPosition+=1;
				continue;
			} else{
				blackLineStop = currentPosition;
				break;
			}
		} else if ( isBlackLine && blackLineStart == 0 ){
			blackLineMode = true;
			blackLineStart = currentPosition;
		}
		currentPosition+=1;
	}

	size_t leftValue = blackLineStart;
	size_t rightValue = imgRawBufSize.width - blackLineStop;

	currentPosition = 0;
	blackLineStart = 0;
	blackLineStop = 0;
	blackLineMode = false;
	for(size_t i=0; i<imgRawBufSize.dataSize; i+=(imgRawBufSize.width*BGRA_BYTE)){

		bool isBlackLine = (memcmp(imgRawBuf+i, NINEGRID_BLACK_LINE, BGRA_BYTE)==0);

		if (blackLineMode){
			if (isBlackLine){
				currentPosition+=1;
				continue;
			} else{
				blackLineStop = currentPosition;
				break;
			}
		} else if ( isBlackLine && blackLineStart == 0 ){
			blackLineMode = true;
			blackLineStart = currentPosition;
		}
		currentPosition+=1;
	}

	size_t topValue = blackLineStart;
	size_t bottomValue = imgRawBufSize.heigh - blackLineStop;

	nineGridInfo.Left = leftValue -1;
	nineGridInfo.Rright = rightValue-1;
	nineGridInfo.Top = topValue-1;
	nineGridInfo.Bottom = bottomValue-1;

	return true;
}

// __out ImageSize &newImgBufSize
bool NinegridCalculatorImpl::CalculateImageSizeWithoutNineGridInfo(ImageSize srcImgRawBufSize,  __out ImageSize &newImgBufSize) {
	const size_t BGRA_BYTE = 4;

	// calculate new size
	ImageSize newImageSize = {0};
	newImgBufSize.dataSize = srcImgRawBufSize.dataSize - ( srcImgRawBufSize.width*2*BGRA_BYTE + ((srcImgRawBufSize.heigh-2)*2*BGRA_BYTE) );
	newImgBufSize.width = srcImgRawBufSize.width - 2;
	newImgBufSize.heigh = srcImgRawBufSize.heigh - 2;

	return true;
}

bool NinegridCalculatorImpl::Trim9GridInfo(BYTE *srcImgRawBuf, ImageSize &srcImgRawBufSize, ImageSize newImgRawBufSizeWithout9GridInfo, __out BYTE **newImageBufferWithout9GridInfo) {
	const size_t BGRA_BYTE = 4;

	*newImageBufferWithout9GridInfo = new BYTE[newImgRawBufSizeWithout9GridInfo.dataSize];
	unsigned long firstLineRange = srcImgRawBufSize.width*4;
	unsigned long currentPoint = -1;
	unsigned long newBufferPosition = 0;
	for(unsigned long i=0; i<srcImgRawBufSize.dataSize; i+=4){
		currentPoint += 1;
		// first line range
		if (i < firstLineRange) continue;
		// left line range
		if (currentPoint%srcImgRawBufSize.width == 0) continue;
		// right line range
		if ((currentPoint+1)%srcImgRawBufSize.width == 0) continue;
		// bottom line range
		if (currentPoint >= (srcImgRawBufSize.width *(srcImgRawBufSize.heigh-1))) break;

		memcpy(*newImageBufferWithout9GridInfo+newBufferPosition, srcImgRawBuf+i, BGRA_BYTE);
		newBufferPosition += 4;
	}
	return true;
}

bool NinegridCalculatorImpl::SaveImageWithout9GridInfo(const string& fileName, BYTE *srcImgRawBuf, ImageSize &srcImgRawBufSize, ImageSize newImgRawBufSizeWithout9GridInfo){
	const size_t BGRA_BYTE = 4;

	BYTE* imageBufferWithout9GridInfo = new BYTE[newImgRawBufSizeWithout9GridInfo.dataSize];
	unsigned long firstLineRange = srcImgRawBufSize.width*4;
	unsigned long currentPoint = -1;
	unsigned long newBufferPosition = 0;
	for(unsigned long i=0; i<srcImgRawBufSize.dataSize; i+=4){
		currentPoint += 1;
		// first line range
		if (i < firstLineRange) continue;
		// left line range
		if (currentPoint%srcImgRawBufSize.width == 0) continue;
		// right line range
		if ((currentPoint+1)%srcImgRawBufSize.width == 0) continue;
		// bottom line range
		if (currentPoint >= (srcImgRawBufSize.width *(srcImgRawBufSize.heigh-1))) break;

		memcpy(imageBufferWithout9GridInfo+newBufferPosition, srcImgRawBuf+i, BGRA_BYTE);
		newBufferPosition += 4;
	}

	// -----------------------------------
	ofstream outfile(fileName.c_str(), ios::binary);
	bool result = false;
	if (outfile.is_open()) {
		PNGBuffer srcPNGRawBuffer;
		PNGBuffer dstPNGEncodedBuffer;
		PNGCodec pngcodec;

		srcPNGRawBuffer.wrap(imageBufferWithout9GridInfo, newImgRawBufSizeWithout9GridInfo.dataSize);
		PNGCodec::ENCODE_ERROR_NO errNo = pngcodec.encode(dstPNGEncodedBuffer, srcPNGRawBuffer, newImgRawBufSizeWithout9GridInfo.width, newImgRawBufSizeWithout9GridInfo.heigh);
		if (errNo != PNGCodec::ENCODE_OK){
		} else{
			outfile.write((char*)dstPNGEncodedBuffer.toPNGBytes(), dstPNGEncodedBuffer.size());
		}


		srcPNGRawBuffer.unwrap();
		dstPNGEncodedBuffer.unwrap();
		result = true;
	}

	delete [] imageBufferWithout9GridInfo;
	return result;
}

bool NinegridCalculatorImpl::SaveRawBufToPngFormat(const std::string& fileName, BYTE *srcImgRawBuf, ImageSize &srcImgRawBufSize){

	ofstream outfile(fileName, ios::binary);

	bool result = false;
	if (outfile.is_open()){
		PNGBuffer srcPNGRawBuffer;
		PNGBuffer dstPNGEncodedBuffer;
		PNGCodec pngcodec;

		srcPNGRawBuffer.wrap(srcImgRawBuf, srcImgRawBufSize.dataSize);
		PNGCodec::ENCODE_ERROR_NO errorNo = pngcodec.encode(dstPNGEncodedBuffer, srcPNGRawBuffer, srcImgRawBufSize.width, srcImgRawBufSize.heigh);
		if (errorNo != PNGCodec::ENCODE_OK){
			result = false;
		} else {			
			outfile.write((char*)dstPNGEncodedBuffer.toPNGBytes(), dstPNGEncodedBuffer.size());
			result = true;
		}

		srcPNGRawBuffer.unwrap();
		dstPNGEncodedBuffer.unwrap();
	} else {
		cerr << "Open file " << fileName << " fail." << endl;
	}

	return result;
}

string NinegridCalculatorImpl::GetBaseFileName(string fileName) {
	string fName(fileName);
	size_t pos = fName.rfind(".");
	if (pos == string::npos) // no extension.
		return fName;
	if (pos == 0) // . is at the front. Not an extension.
		return fName;
	return fName.substr(0, pos);
}

vector<string> NinegridCalculatorImpl::GetPngFileListAtFolder(const std::string &searchKey){
	vector<string> result;

	GetFileList(searchKey, result);

	return result;
}

vector<size_t> NinegridCalculatorImpl::GetSplitRawBufferSizeByHorizontal(ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo){
	size_t firstRow = ninegridInfo.Top;
	size_t secondRow = srcImageRawBufferSize.heigh - ninegridInfo.Top - ninegridInfo.Bottom;
	size_t thirdRow = srcImageRawBufferSize.heigh - firstRow - secondRow;

	size_t buffer1Size = firstRow * 4 * srcImageRawBufferSize.width;
	size_t buffer2Size = secondRow * 4 * srcImageRawBufferSize.width;
	size_t buffer3Size = thirdRow * 4 * srcImageRawBufferSize.width;

	vector<size_t> result;
	result.push_back(buffer1Size);
	result.push_back(buffer2Size);
	result.push_back(buffer3Size);

	return result;
}

vector<BYTE*> NinegridCalculatorImpl::GetSplitRawBufferByHorizontal(BYTE *srcImageRawBuffer, ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo){
	vector<size_t> buffersSize = GetSplitRawBufferSizeByHorizontal(srcImageRawBufferSize, ninegridInfo);
	size_t buffer1Size = buffersSize.at(0);
	size_t buffer2Size = buffersSize.at(1);
	size_t buffer3Size = buffersSize.at(2);

	BYTE* buffer1 = (BYTE*)allocMemory(buffer1Size);
	BYTE* buffer2 = (BYTE*)allocMemory(buffer2Size);
	BYTE* buffer3 = (BYTE*)allocMemory(buffer3Size);

#ifdef DEBUG
	assert ((buffer1Size + buffer2Size + buffer3Size) == srcImageRawBufferSize.dataSize);
#endif

	memcpy(buffer1, srcImageRawBuffer, buffer1Size);
	memcpy(buffer2, srcImageRawBuffer+buffer1Size, buffer2Size);
	memcpy(buffer3, srcImageRawBuffer+buffer1Size+buffer2Size, buffer3Size);

	vector<BYTE*> result;
	result.push_back(buffer1);
	result.push_back(buffer2);
	result.push_back(buffer3);

	return result;
}

vector<size_t> NinegridCalculatorImpl::GetSplitRawBufferSizeByVertical(ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo){
	vector<size_t> result; 

	size_t leftRegionSize = ninegridInfo.Left;
	size_t middleRegionSize = srcImageRawBufferSize.width - ninegridInfo.Left - ninegridInfo.Rright;
	size_t rightRegionSize = srcImageRawBufferSize.width - leftRegionSize - middleRegionSize;

	size_t leftBufferSize = leftRegionSize * srcImageRawBufferSize.heigh * 4;
	size_t middleBufferSize = middleRegionSize * srcImageRawBufferSize.heigh * 4;
	size_t rightBufferSize = rightRegionSize * srcImageRawBufferSize.heigh * 4;
	
	result.push_back(leftBufferSize);
	result.push_back(middleBufferSize);
	result.push_back(rightBufferSize);

	return result;
}

vector<BYTE*> NinegridCalculatorImpl::GetSplitRawBufferByVertical(BYTE *srcImageRawBuffer, ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo){
	vector<BYTE*> result;
	
	vector<size_t> bufferSize = GetSplitRawBufferSizeByVertical(srcImageRawBufferSize, ninegridInfo);
	size_t leftBufferSize = bufferSize.at(0);
	size_t middleBufferSize = bufferSize.at(1);
	size_t rightBufferSize = bufferSize.at(2);

	BYTE* leftBuffer = (BYTE*)allocMemory(leftBufferSize);
	BYTE* middleBuffer = (BYTE*)allocMemory(middleBufferSize);
	BYTE* rightBuffer = (BYTE*)allocMemory(rightBufferSize);

	size_t leftBufferSizeIndex = 0;
	size_t middleBufferSizeIndex = 0;
	size_t rightBufferSizeIndex = 0;

	size_t leftBufferDataWidth = ninegridInfo.Left * 4;
	size_t rightBufferDataWidth = ninegridInfo.Rright * 4;
	size_t middleBufferDataWidth = srcImageRawBufferSize.width*4 - leftBufferDataWidth - rightBufferDataWidth;

	const size_t leftStartIndex = 0;
	const size_t middleStartIndex =  leftBufferDataWidth;
	const size_t rightStartIndex = leftBufferDataWidth + middleBufferDataWidth;

	size_t copySize = 0;
	for (size_t rawBufferIndex = 0; rawBufferIndex < srcImageRawBufferSize.dataSize; ) {
		size_t baseIndex = rawBufferIndex % (srcImageRawBufferSize.width*4);

		if (baseIndex==leftStartIndex) {
			copySize = leftBufferDataWidth;
			memcpy(leftBuffer+leftBufferSizeIndex, srcImageRawBuffer+rawBufferIndex, copySize);
			leftBufferSizeIndex += copySize;
			rawBufferIndex += copySize;
			continue;
		} 
		if (baseIndex == middleStartIndex){
			copySize = middleBufferDataWidth;
			memcpy(middleBuffer+middleBufferSizeIndex, srcImageRawBuffer+rawBufferIndex, copySize);
			middleBufferSizeIndex += copySize;
			rawBufferIndex += copySize;
			continue;
		}
		if (baseIndex == rightStartIndex ) {
			copySize = rightBufferDataWidth;
			memcpy(rightBuffer+rightBufferSizeIndex, srcImageRawBuffer+rawBufferIndex, copySize);
			rightBufferSizeIndex += copySize;
			rawBufferIndex += copySize;
			continue;
		}

		break;
	}

	result.push_back(leftBuffer);
	result.push_back(middleBuffer);
	result.push_back(rightBuffer);
	
	return result;
}

// return datasize sequence by
//  0 1 2
//  3 4 5
//  6 7 8
vector<size_t> NinegridCalculatorImpl::GetSplitRawBufferSizeBy9Grid(ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo){
	vector<size_t> result;
	
	size_t leftWidth = ninegridInfo.Left * 4;
	size_t rightWidth = ninegridInfo.Rright * 4;
	size_t middleWidth = (srcImageRawBufferSize.width*4) - leftWidth - rightWidth;
	size_t topHeight = ninegridInfo.Top;
	size_t bottomHeight = ninegridInfo.Bottom;
	size_t middleHeight = (srcImageRawBufferSize.heigh) - topHeight - bottomHeight;
	
	result.push_back(size_t(leftWidth*topHeight));
	result.push_back(size_t(middleWidth*topHeight));
	result.push_back(size_t(rightWidth*topHeight));
	result.push_back(size_t(leftWidth*middleHeight));
	result.push_back(size_t(middleWidth*middleHeight));
	result.push_back(size_t(rightWidth*middleHeight));
	result.push_back(size_t(leftWidth*bottomHeight));
	result.push_back(size_t(middleWidth*bottomHeight));
	result.push_back(size_t(rightWidth*bottomHeight));

	return result;
}
vector<BYTE*> NinegridCalculatorImpl::GetSplitRawBufferBy9Grid(BYTE* srcImageRawBuffer, ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo){
	vector<BYTE*> result;

	vector<size_t> buffer3HSize = GetSplitRawBufferSizeByHorizontal(srcImageRawBufferSize, ninegridInfo);
	vector<BYTE*> buffers3HAll = GetSplitRawBufferByHorizontal(srcImageRawBuffer, srcImageRawBufferSize, ninegridInfo);

	size_t topHeight = ninegridInfo.Top;
	size_t bottomHeight = ninegridInfo.Bottom;
	size_t middleHieght = srcImageRawBufferSize.heigh - topHeight - bottomHeight;

	ImageSize imageSize;
	imageSize.width = srcImageRawBufferSize.width;
	imageSize.heigh = topHeight;
	imageSize.dataSize = (imageSize.width*4) * (imageSize.heigh);
	vector<BYTE*> buffer3V1 = GetSplitRawBufferByVertical(buffers3HAll.at(0), imageSize, ninegridInfo);
	
	imageSize.heigh = middleHieght;
	imageSize.dataSize = imageSize.dataSize = (imageSize.width*4) * (imageSize.heigh);
	vector<BYTE*> buffer3V2 = GetSplitRawBufferByVertical(buffers3HAll.at(1), imageSize, ninegridInfo);

	imageSize.heigh = bottomHeight;
	imageSize.dataSize = imageSize.dataSize = (imageSize.width*4) * (imageSize.heigh);
	vector<BYTE*> buffer3V3 = GetSplitRawBufferByVertical(buffers3HAll.at(2), imageSize, ninegridInfo);

	result.push_back(buffer3V1.at(0));
	result.push_back(buffer3V1.at(1));
	result.push_back(buffer3V1.at(2));
	
	result.push_back(buffer3V2.at(0));
	result.push_back(buffer3V2.at(1));
	result.push_back(buffer3V2.at(2));

	result.push_back(buffer3V3.at(0));
	result.push_back(buffer3V3.at(1));
	result.push_back(buffer3V3.at(2));

	for(size_t i = 0; i<buffers3HAll.size(); i++)
		delete [] buffers3HAll.at(i);

	return result;
}


// private method
void* NinegridCalculatorImpl::allocMemory(size_t size){
	try {
		void *Memory = ::operator new (size);
		return Memory;
	} catch(...){}
	return NULL;
}

#ifdef WIN32
wstring NinegridCalculatorImpl::StringToWideString(const string& as) {
	if( as.empty() )   
		return std::wstring();

	size_t reqLength = ::MultiByteToWideChar( CP_UTF8, 0, as.c_str(), (int)as.length(), 0, 0 );
	std::wstring ret( reqLength, L'\0' );
	::MultiByteToWideChar( CP_UTF8, 0, as.c_str(), (int)as.length(), &ret[0], (int)ret.length() );
	return ret;
}

string NinegridCalculatorImpl::WideStringToString(const std::wstring& wideString) {
	static char buffer[MAX_PATH];
	if (0 == ::WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, buffer, MAX_PATH, NULL, NULL))
		return std::string("");
	return std::string(buffer);
}
#endif


bool NinegridCalculatorImpl::has_suffix(const string& s, const string& suffix) {
	return ( s.size() >= suffix.size() ) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}


int NinegridCalculatorImpl::GetFileList(string searchkey, std::vector<std::string> &list) {
	vector<string> result;


#ifdef WIN32
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(StringToWideString(searchkey).c_str(), &fd);

	if (h==INVALID_HANDLE_VALUE)
		return 0;

	while(true){
		list.push_back(WideStringToString(fd.cFileName));
		if(FindNextFile(h, &fd) == FALSE)
			break;
	}
#else
	char cCurrentPath[FILENAME_MAX];
	getCurrentDir(cCurrentPath, sizeof(cCurrentPath));

	DIR *dir = opendir(cCurrentPath);
	if(!dir){ return 1;}
	dirent *entry;
	while(entry = readdir(dir)) {
		if ( has_suffix(entry->d_name, ".png")){
			cout << "   " <<  entry->d_name << endl;
			list.push_back(string(entry->d_name));
		}
	}
#endif
	return list.size();
}