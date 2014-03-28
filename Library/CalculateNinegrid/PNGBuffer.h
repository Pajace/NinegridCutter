#ifndef HTC_APOLLO_PNG_BUFFER_H_
#define HTC_APOLLO_PNG_BUFFER_H_
#include "png.h"

namespace hTC {

	namespace ComposerLib {

		class PNGBuffer {

		public:
			PNGBuffer(void);
			~PNGBuffer(void);

			const bool append(const png_bytep content, const png_size_t content_size, int failedTestIndex=-1);
			const bool clear(void);
			const png_size_t size(void) const;
			const png_bytep toPNGBytes(void) const;
			void unwrap(void);
			void wrap(const png_bytep content, const png_size_t content_size);

			static void* AllocMemory(png_size_t size, bool isFailedTest);
		private:
			png_bytep mContent;
			png_size_t mContentBytes;
			png_size_t mAllocatedBytes;
			bool mIsWrapper;

			void detroy(void);
		}; // PNGBuffer

	} // Apollo
} // hTC

#endif // HTC_APOLLO_PNG_BUFFER_H_