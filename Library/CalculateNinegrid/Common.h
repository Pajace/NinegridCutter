#ifndef HTC_IMAGE_NINEGRID_COMMON_H_
#define HTC_IMAGE_NINEGRID_COMMON_H_

#define WINDOWS

#ifdef WINDOWS
#include <Windows.h>
#else
#endif

namespace hTC { namespace Image { namespace Ninegrid {
	struct ImageSize{
		unsigned long width;
		unsigned long heigh;
		unsigned long dataSize; // array size
	};

	struct NineGridInfo {
		long Left;
		long Rright;
		long Top;
		long Bottom;
	};
}}}

#endif // HTC_IMAGE_NINEGRID_COMMON_H_