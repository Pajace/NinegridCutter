#ifndef HTC_IMAGE_NINEGRID_NINEGRIDCALCULATORIMPL_H_
#define HTC_IMAGE_NINEGRID_NINEGRIDCALCULATORIMPL_H_

#include "Common.h"
#include <string>
#include <vector>

namespace hTC { namespace Image { namespace Ninegrid {

	class NinegridCalculatorImpl
	{
	public:
		NinegridCalculatorImpl(void);
		~NinegridCalculatorImpl(void);

		// Get PNG raw buffer image
		// ------------------------
		// @filename: file name of PNG image.
		// @imgRawBuf: (out parameter) the raw buffer of PNG image
		// @pngRawBufImageSize: (out parameter) the image size of PNG image
		// Return true: success otherwise fail
		bool GetPngRawBuffer(const std::string& filename, __out BYTE **imgRawBuf, __out ImageSize &pngRawBufImgSize);

		// Get NineGridInfo
		// -------------------
		// @imgRawBuf: image raw buffer
		// @imgRawBufSize: thw raw buffer size of imgRawBuf
		// @nineGridInfo: (out parameter) the result Nine grid information
		// return true if get info success otherwis is fail
		bool CalculateNineGridInfo(BYTE *imgRawBuf, ImageSize &imgRawBufSize, __out NineGridInfo &nineGridInfo);

		bool CalculateImageSizeWithoutNineGridInfo(ImageSize srcImgRawBufSize,  __out ImageSize &newImgBufSize);

		bool SaveImageWithout9GridInfo(const std::string& fileName, BYTE *srcImgRawBuf, ImageSize &srcImgRawBufSize, ImageSize newImgRawBufSizeWithout9GridInfo);
		bool Trim9GridInfo(BYTE *srcImgRawBuf, ImageSize &srcImgRawBufSize, ImageSize newImgRawBufSizeWithout9GridInfo, __out BYTE **newImageBufferWithout9GridInfo);

		// encode @srcImgRawBuf to png buffer and save to @fileName
		bool SaveRawBufToPngFormat(const std::string& fileName, BYTE *srcImgRawBuf, ImageSize &srcImgRawBufSize);

		std::vector<size_t> GetSplitRawBufferSizeByHorizontal(ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo);
		std::vector<BYTE*> GetSplitRawBufferByHorizontal(BYTE *srcImageRawBuffer, ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo);

		std::vector<size_t> GetSplitRawBufferSizeByVertical(ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo);
		std::vector<BYTE*> GetSplitRawBufferByVertical(BYTE *srcImageRawBuffer, ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo);

		std::vector<size_t> GetSplitRawBufferSizeBy9Grid(ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo);
		std::vector<BYTE*> GetSplitRawBufferBy9Grid(BYTE* srcImageRawBuffer, ImageSize &srcImageRawBufferSize, NineGridInfo &ninegridInfo);

		std::vector<std::string> GetPngFileListAtFolder(const std::string &searchKey);

		std::string GetBaseFileName(std::string fileName);

	private:
		//BYTE* baseImg;
		//ImageSize baseImageSize;
		//NineGridInfo ninegrid;

		void* allocMemory(size_t size);
		static std::wstring StringToWideString(const std::string& as);
		static std::string WideStringToString(const std::wstring& wideString);		
		int GetFileList(std::string searchkey, std::vector<std::string> &list);
	};

}}} // end hTC::Image::Ninegrid

#endif // HTC_IMAGE_NINEGRID_NINEGRIDCALCULATORIMPL_H_