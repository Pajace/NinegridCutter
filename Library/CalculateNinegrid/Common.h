#ifndef HTC_IMAGE_NINEGRID_COMMON_H_
#define HTC_IMAGE_NINEGRID_COMMON_H_

#ifdef WIN32
	#include <Windows.h>
#else
	typedef unsigned char BYTE;
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

#ifdef WIN32
#define FOLDER_SEPARATOR  "\\"
#else
#define FOLDER_SEPARATOR  "/"
#endif // !WIN32


}}}

#endif // HTC_IMAGE_NINEGRID_COMMON_H_