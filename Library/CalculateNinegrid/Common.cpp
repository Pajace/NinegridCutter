#include "Common.h"



#include <windows.h>
#include <string>
#include "Common.h"

namespace hTC {
	namespace ComposerLib{

		class ComposerImpl;

		class Composer
		{
		public:
			Composer(void);
			~Composer(void);

			// Create an empty image
			// 32bit (BGRA)
			bool CreateBaseImage(long width, long height);

			// Get PNG raw buffer image
			// ------------------------
			// @fileName: file name of PNG image.
			// @imgRawBuf: (out parameter) the raw buffer of PNG image
			// @pngRawBufImgSize: (out parameter) the image size of PNG image
			// Return: true if success
			bool Composer::GetPngRawBuffer(const std::string& fileName, __out BYTE **imgRawBuf, __out ImageSize &pngRawBufImgSize);

			// Add part on base image
			// ----------------------
			// @pngRawBuf: the raw format of PNG image
			// @pngRawBufImageSize: the size of PNG image
			// @position: the position on base image
			// Return: true if success
			bool AddPartOnBaseImage(BYTE* pngRawBuf, ImageSize pngRawBufImgSize, POINT position);

			// Add part on base image
			// ----------------------
			// @fileName: file name of PNG image.
			// @position: the position on based image.
			// Return: true if success
			bool AddPartOnBaseImage(const std::string& fileName, POINT position);
 
			// Output the merged image to file
			// -------------------------------
			// @fileName: file name of output PNG image
			// Return: true if success
			bool SaveResult2PngFile(const std::string& fileName);

			// Get the PNG-encoded buffer and size
			// -------------------------------
			// @buf: (out parameter) the encoded buffer
			// @bufSize: (out parameter) the encoded buffer size
			// Return: true if success
			bool GetEncodedBuffer(__out BYTE** buf, __out unsigned long* bufSize);

		private:
			ComposerImpl *_pimpl;
		};	
	}
}