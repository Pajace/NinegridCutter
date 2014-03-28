#ifndef HTC_IMAGE_NINEGRID_NINEGRIDCALCULATOR_H_
#define HTC_IMAGE_NINEGRID_NINEGRIDCALCULATOR_H_

#include <string>
#include <vector>
#include "Common.h"

namespace hTC { namespace Image { namespace Ninegrid {

	class NinegridCalculatorImpl;

	class NinegridCalculator
	{
	public:
		NinegridCalculator();
		~NinegridCalculator(void);

		bool RunCalculaterAndOutpuInfo();
		bool RunSplitImageWith3V();
		bool RunSplitImageWith3H();
		bool RunSplitImageWith9Grid();

	private:
		NinegridCalculatorImpl *_pimpl;

		bool createFolder(std::string folderPath);
		std::vector<std::string> get9GridPngFileList();
		std::string getSecondayBaseFileName(std::string filename);

	};

}}} // hTC::Image::Ninegrid

#endif // HTC_IMAGE_NINEGRID_NINEGRIDCALCULATOR_H_