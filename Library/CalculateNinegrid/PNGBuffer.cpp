#include "png.h"
#include "PNGBuffer.h"
#include "Common.h"
#include <new>

using namespace std;

namespace hTC {
	namespace ComposerLib {

		PNGBuffer::PNGBuffer(void) :
			mContent(NULL),
			mContentBytes(0),
			mAllocatedBytes(0),
			mIsWrapper(false) {
		}

		PNGBuffer::~PNGBuffer(void) {
			this->detroy();
		}

//#pragma region Public Members

		const bool PNGBuffer::append(const png_bytep content, const png_size_t content_size, int failedTestIndex) {

			if (this->mIsWrapper) return false;

			if (0 == content_size) return true;

			if (NULL == content) return false;
			png_size_t bufferSize = this->size();
			png_size_t neededSpace = bufferSize + content_size;
#ifdef WIN32
			errno_t err = 0;
#endif

			if (neededSpace > this->mAllocatedBytes) {
				png_bytep backup = NULL;

				if (0 < bufferSize && this->mContent!=NULL) {
					backup = (png_bytep)AllocMemory(bufferSize, (failedTestIndex==0));
					if (backup==NULL) {  
						return false; 
					}
#ifdef WIN32
					err = memcpy_s(backup, bufferSize, this->mContent, bufferSize);
					if (0 != err) {
						delete[] backup;
						backup = NULL;
						return false;
					}
#else
					memcpy(backup, this->mContent, bufferSize);
#endif


					delete[] this->mContent;
				}

				this->mContent = (png_bytep)AllocMemory(neededSpace, (failedTestIndex==1));
				if (mContent==NULL) {
					if (backup!=NULL) {
						delete [] backup; 
						backup = NULL;
					}
					return false; 
				}

				this->mAllocatedBytes = neededSpace;

				if (0 < bufferSize) {
#ifdef WIN32
					err = memcpy_s(this->mContent, neededSpace, backup, bufferSize);
#else
					memcpy(this->mContent, backup, bufferSize);
#endif
					delete[] backup;
					backup = NULL;
#ifdef WIN32
					if (0 != err) return false;
#endif
				}
			}
#ifdef WIN32
			err = memcpy_s(this->mContent + bufferSize, this->mAllocatedBytes - bufferSize, content, content_size);
			if (0 != err) return false;
#else
			memcpy(this->mContent+bufferSize, content, content_size);
#endif
			this->mContentBytes = neededSpace;
			return true;
		}

		const bool PNGBuffer::clear(void) {

			if (this->mIsWrapper) return false;
			this->mContentBytes = 0;
			return true;
		}

		const png_size_t PNGBuffer::size(void) const {
			return this->mContentBytes;
		}

		const png_bytep PNGBuffer::toPNGBytes(void) const {
			return this->mContent;
		}

		void PNGBuffer::unwrap(void) {
			this->detroy();
			this->mIsWrapper = false;
		}

		void PNGBuffer::wrap(const png_bytep content, const png_size_t content_size) {
			this->detroy();
			this->mContent = content;
			this->mContentBytes = content_size;
			this->mIsWrapper = true;
		}

		void* PNGBuffer::AllocMemory(png_size_t size, bool isFailedTest){
			if (isFailedTest)
				return NULL;
			else {
				try {
					void *Memory = ::operator new (size);
					return Memory;
				} catch (...) {
				}
				return NULL;
			}
		}
//#pragma endregion

//#pragma region Private Members

		void PNGBuffer::detroy(void) {

			if (!this->mIsWrapper && NULL != this->mContent) {
				delete[] this->mContent;
				this->mAllocatedBytes = 0;
			}
			this->mContent = NULL;
			this->mContentBytes = 0;
		}
//#pragma endregion

	} // apollo
} // hTC