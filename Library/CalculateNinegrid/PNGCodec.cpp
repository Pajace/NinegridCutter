#include "PNGCodec.h"
#include "PNGBuffer.h"

using namespace std;

#define  UNREFERENCED_PARAMETER(P) (P)

namespace hTC {

	namespace ComposerLib {

		PNGCodec::PNGCodec(void) {
		}

		PNGCodec::~PNGCodec(void) {
		}

#pragma region Public Members

		const PNGCodec::ENCODE_ERROR_NO PNGCodec::encode(
			__out PNGBuffer &dest_buffer,
			__in  const PNGBuffer &src_buffer,
			__in  const png_uint_32 width,
			__in  const png_uint_32 height
			) const {
				png_structp png_ptr = png_create_write_struct(
					PNG_LIBPNG_VER_STRING,
					NULL, NULL, NULL
					);

				if (NULL == png_ptr) return ENCODE_ERROR_NO::FAILED_TO_CREATE_PNG_STRUCTURE;
				png_infop info_ptr = png_create_info_struct(png_ptr);

				if (NULL == info_ptr) {
					png_destroy_write_struct(&png_ptr, NULL);
					return ENCODE_ERROR_NO::FAILED_TO_CREATE_PNG_INFO_STRUCTURE;
				}
				png_set_write_fn(
					png_ptr,
					&dest_buffer,
					(png_rw_ptr)hTC::ComposerLib::PNGCodec::appendPNGBuffer,
					(png_flush_ptr)hTC::ComposerLib::PNGCodec::flushPNGBuffer
					);



				png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

				/* Write the file header information.  REQUIRED */
				png_write_info(png_ptr, info_ptr);
				png_bytepp imgRows = new png_bytep[height];
				if (imgRows==NULL) {
					return ENCODE_ERROR_NO::FAILED_TO_ALLOCAT_MEMORY;
				}

				for (png_uint_32 i = 0; i < height; ++ i) {
					imgRows[i] = src_buffer.toPNGBytes() + 4 * (width * i) * sizeof png_byte;
				}
				png_write_image(png_ptr, imgRows);
				png_write_end(png_ptr, info_ptr);

				if (NULL != imgRows) {
					delete[] imgRows;
					imgRows = NULL;
				}
				png_destroy_write_struct(&png_ptr, &info_ptr);
				return ENCODE_ERROR_NO::ENCODE_OK;
		}

		const PNGCodec::ENCODE_ERROR_NO PNGCodec::readPNGFromPath(
			__out PNGBuffer &dest_buffer,
			__out png_uint_32 &o_width,
			__out png_uint_32 &o_height,
			__in const char *path
			) const {
				FILE *fp = NULL;

				if (0 != fopen_s(&fp, path, "rb")) {
					fprintf(stderr, "can't open %s\n", path);
					return ENCODE_ERROR_NO::FAILED_TO_OPEN_FILE;
				}

				png_structp png_ptr = png_create_read_struct(
					PNG_LIBPNG_VER_STRING,
					NULL, NULL, NULL
					);

				if (NULL == png_ptr) return ENCODE_ERROR_NO::FAILED_TO_CREATE_PNG_STRUCTURE;
				png_infop info_ptr = png_create_info_struct(png_ptr);

				if (NULL == info_ptr) {
					png_destroy_read_struct(&png_ptr, NULL, NULL);
					return ENCODE_ERROR_NO::FAILED_TO_CREATE_PNG_INFO_STRUCTURE;
				}
				png_init_io(png_ptr, fp);
				png_read_info(png_ptr, info_ptr);
				png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
				png_read_update_info(png_ptr, info_ptr);
				png_size_t rowBytes = png_get_rowbytes(png_ptr,info_ptr);
				png_bytepp image = new png_bytep[height];
				if (image==NULL) {
					return ENCODE_ERROR_NO::FAILED_TO_ALLOCAT_MEMORY;
				}

				o_width = png_get_image_width(png_ptr, info_ptr);
				o_height = height;

				bool isAnyRowNull = false;
				for (png_uint_32 i = 0; i < height; ++ i) {
					image[i] = new png_byte[rowBytes];
					if (image[i] == NULL) {
						isAnyRowNull = true;
						break;
					}
				}
				if (isAnyRowNull) {
					fclose(fp);
					if (image != NULL) {
						for(png_uint_32 i = 0; i < height; ++ i) {
							if (image[i]!=NULL)  delete [] image[i];
						}
						if (image != NULL) { delete [] image; }
					}					
					return ENCODE_ERROR_NO::FAILED_TO_ALLOCAT_MEMORY;
				}


				png_read_image(png_ptr, image);
				png_read_end(png_ptr, info_ptr);
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				fclose(fp);

				for (png_uint_32 i = 0; i < height; ++ i) {
					dest_buffer.append(image[i], rowBytes);
				}

				if (NULL != image) {

					for (png_uint_32 i = 0; i < height; ++ i) {

						if (NULL != image[i]) {
							delete[] image[i];
							image[i] = NULL;
						}
					}
					delete[] image;
					image = NULL;
				}
				return ENCODE_ERROR_NO::ENCODE_OK;
		}

		const bool PNGCodec::writePNGToPath(__in const char *path) const {
			const int image_height = 5;	/* Number of rows in image */
			const int image_width = 5;	/* Number of columns in image */
			png_bytep image = new  png_byte[image_height * image_width * 4];
			if (image == NULL) {
				return false;
			}

			const unsigned char tmp[] = {0x00, 0x00, 0xFF, 0xFF};
			for (int i = 0; i < image_height; ++ i) {
				for (int j = 0; j < image_width; j+=4){
					memcpy(image+(i * image_width * 4 + 4 * j), tmp, 4);
				}
			}
			PNGBuffer srcBuffer;
			srcBuffer.append(image, image_height * image_width * 4);
			PNGBuffer destBuffer;
			this->encode(destBuffer, srcBuffer, image_width, image_height);
			FILE *fp = NULL;

			if (0 != fopen_s(&fp, path, "wb")) {
				fprintf(stderr, "can't open %s\n", path);
				return false;
			}
			fwrite(destBuffer.toPNGBytes(), 1, destBuffer.size(), fp);
			fclose(fp);

			if (NULL != image) {
				delete[] image;
				image = NULL;
			}
			return true;
		}

#pragma endregion

#pragma region Private Members

		void PNGCodec::appendPNGBuffer(png_structp png_ptr, png_bytep data, png_size_t length) {
			PNGBuffer *bufferPtr = reinterpret_cast<PNGBuffer*>(png_ptr->io_ptr);
			bufferPtr->append(data, length);
		}

		void PNGCodec::flushPNGBuffer(png_structp png_ptr) {
			UNREFERENCED_PARAMETER(png_ptr);
		}

#pragma endregion

	} // Composer
} // hTC