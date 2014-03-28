#ifndef HTC_APOLLO_PNG_CODEC_H_
#define HTC_APOLLO_PNG_CODEC_H_

#include "PNGBuffer.h"


namespace hTC {
	namespace ComposerLib {
		class PNGCodec {

		public:

			enum ENCODE_ERROR_NO {
				ENCODE_OK,
				FAILED_TO_CREATE_PNG_STRUCTURE,
				FAILED_TO_CREATE_PNG_INFO_STRUCTURE,
				FAILED_TO_OPEN_FILE,
				FAILED_TO_ALLOCAT_MEMORY,
			};

			PNGCodec(void);
			~PNGCodec(void);

			// input RGBA buffer & get PNG stream buffer
			const ENCODE_ERROR_NO encode(
				__out PNGBuffer &dest_buffer,
				__in  const PNGBuffer &src_buffer,
				__in  const png_uint_32 width,
				__in  const png_uint_32 height
				) const;

			// read PNG file & get RGBA buffer
			const ENCODE_ERROR_NO readPNGFromPath(
				__out PNGBuffer &dest_buffer,
				__out png_uint_32 &width,
				__out png_uint_32 &height,
				__in const char *path
				) const;

			const bool writePNGToPath(__in const char *path) const;

		private:
			static void appendPNGBuffer(png_structp png_ptr, png_bytep data, png_size_t length);
			static void flushPNGBuffer(png_structp png_ptr);
		}; // PNGCodec
	} // ComposerLib
} // hTC

#endif // HTC_APOLLO_PNG_CODEC_H_