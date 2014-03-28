#ifndef HTC_IMAGE_NINEGRID_NINEGRIDCALCULATOR_H_
#define HTC_IMAGE_NINEGRID_NINEGRIDCALCULATOR_H_

#include <string>
#include "Common.h"

namespace hTC { namespace Image { namespace Ninegrid {

	class NinegridCalculatorImpl;

	class NinegridCalculator
	{
	public:
		NinegridCalculator();
		~NinegridCalculator(void);

		bool RunCalculaterAndOutpuInfo();
		bool RunSplitImageWith3H();
		bool RunSplitImageWith3V();
		bool RunSplitImageWith9Grid();

	private:
		NinegridCalculatorImpl *_pimpl;

		bool createFolder(std::string folderPath);

	};

}}} // hTC::Image::Ninegrid

#endif // HTC_IMAGE_NINEGRID_NINEGRIDCALCULATOR_H_