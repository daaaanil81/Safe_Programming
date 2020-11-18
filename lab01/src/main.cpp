#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include "png.h"

struct ImageInfo
{
    
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep* row_pointers;
    ImageInfo()
    {
        width = 0;
        height = 0;
        png_ptr = nullptr;
        info_ptr = nullptr;
        row_pointers = nullptr;
    }
    
    ~ImageInfo()
    {
        if (height != 0 || row_pointers != nullptr)
        {
            for (int y = 0; y < height; ++y)
            {
                if (row_pointers[y] != nullptr)
                    delete [] row_pointers[y];
            }
            delete [] row_pointers;
        }
    }
    
};

bool read_png_file(const char* file_name, ImageInfo* image);
bool write_png_file(const char* file_name, ImageInfo* image);

bool colorTest(const char* fileName, const char* fileNameNew)
{
    ImageInfo image;
    if (!read_png_file(fileName, &image))
        return false;
    
    int k = 0;
    int index = 0;
    if (image.color_type == PNG_COLOR_TYPE_RGBA)
        ++index; 

    if (image.color_type == PNG_COLOR_TYPE_RGBA || image.color_type == PNG_COLOR_TYPE_RGB)
    {
        for (int j = 0; j < image.height; ++j)
        {
            k = 0;
            for (int i = 0; i < image.width; ++i)
            {
                if (image.row_pointers[j][k] & 0x1)
                    image.row_pointers[j][k] = 255;
                else
                    image.row_pointers[j][k] = 0;
                
                if (image.row_pointers[j][k+1] & 0x1)
                    image.row_pointers[j][k+1] = 255;
                else
                    image.row_pointers[j][k+1] = 0;
                
                if (image.row_pointers[j][k+2] & 0x1)
                    image.row_pointers[j][k+2] = 255;
                else
                    image.row_pointers[j][k+2] = 0;
                
                k += 3 + index;
            }
        }
    }
    else
    {
        std::cerr << "color_type must be PNG_COLOR_TYPE_RGBA or PNG_COLOR_TYPE_RGB (is " << image.color_type << ")" << std::endl;
        return false;
    }
    
    if (!write_png_file(fileNameNew, &image))
        return false;
    return true;
}

bool read_png_file(const char* file_name, ImageInfo* image)
{
    char header[8];    // 8 is the maximum size that can be checked
    
    if (image == nullptr)
        return false;
    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    std::string error;    
    
    do
    {
        error = std::string("[read_png_file] File %s could not be opened for reading ") + file_name;
        if (!fp)
            break;
            
        fread(header, 1, 8, fp);

        error = std::string("[read_png_file] File %s is not recognized as a PNG file ") + file_name;
        if (png_sig_cmp(reinterpret_cast<png_const_bytep>(header), 0, 8)) // checks whether the given number of bytes match the PNG signature starting from the start position.
            break;

        /* initialize stuff */
        image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); // Create struct png for read

        error = "[read_png_file] png_create_read_struct failed";
        if (!image->png_ptr)
            break;
        
        image->info_ptr = png_create_info_struct(image->png_ptr); // Create struct png with information

        error = "[read_png_file] png_create_info_struct failed";
        if (!image->info_ptr)
            break;

        error = "[read_png_file] Error during init_io";
        if (setjmp(png_jmpbuf(image->png_ptr)))
            break;
        
        png_init_io(image->png_ptr, fp); // Initialize the default input/output functions for the PNG file to standard C streams.
        png_set_sig_bytes(image->png_ptr, 8); //shall store the number of bytes of the PNG file signature that have been read from the PNG stream.

        png_read_info(image->png_ptr, image->info_ptr); // Reads the information before the actual image data from the PNG file. The function allows reading a file that already has the PNG signature bytes read from the stream.

        image->width = png_get_image_width(image->png_ptr, image->info_ptr);
        image->height = png_get_image_height(image->png_ptr, image->info_ptr);
        image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
        image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

        error = std::string("color_type must be PNG_COLOR_TYPE_RGBA or PNG_COLOR_TYPE_RGBv (is ") + std::to_string(image->color_type) + ")";
        if (image->color_type != PNG_COLOR_TYPE_RGBA && image->color_type != PNG_COLOR_TYPE_RGB)
            break;
        
        image->number_of_passes = png_set_interlace_handling(image->png_ptr);
        png_read_update_info(image->png_ptr, image->info_ptr); // Updates the structure pointed to by image->info_ptr to reflect any transformations that have been requested
        

        error = "[read_png_file] Error during read_image";
        if (setjmp(png_jmpbuf(image->png_ptr)))
            break;
    
        image->row_pointers = new png_bytep[sizeof(png_bytep) * image->height];
        for (int y = 0; y < image->height; ++y)
            image->row_pointers[y] = new png_byte[png_get_rowbytes(image->png_ptr,image->info_ptr)]; // Returns the number of bytes needed to hold a transformed row of an image.
        
        png_read_image(image->png_ptr, image->row_pointers); // Read Image
        error = "";
    }
    while(false);
    
    if (fp)
        fclose(fp);
    if(!error.empty())
    {
        std::cerr << error << std::endl;
        return false;
    }
    return true;
}


bool write_png_file(const char* file_name, ImageInfo* image)
{
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    std::string error;
    do
    {
        error = std::string("[write_png_file] File %s could not be opened for writing ") + file_name;
        if (!fp)
            break;
        /* initialize stuff */
        image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        error = "[write_png_file] png_create_write_struct failed";
        if (!image->png_ptr)
            break;
    
        image->info_ptr = png_create_info_struct(image->png_ptr);

        error = "[write_png_file] png_create_info_struct failed";
        if (!image->info_ptr)
            break;

        error = "[write_png_file] Error during init_io";
        if (setjmp(png_jmpbuf(image->png_ptr)))
            break;
        
        png_init_io(image->png_ptr, fp);

        /* write header */
        error = "[write_png_file] Error during writing header";
        if (setjmp(png_jmpbuf(image->png_ptr)))
            break;

        png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                     image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        
        png_write_info(image->png_ptr, image->info_ptr);


        /* write bytes */
        error = "[write_png_file] Error during writing bytes";
        if (setjmp(png_jmpbuf(image->png_ptr)))
            break;
        
        png_write_image(image->png_ptr, image->row_pointers);


        /* end write */
        error = "[write_png_file] Error during end of write";
        if (setjmp(png_jmpbuf(image->png_ptr)))
            break;
        
        png_write_end(image->png_ptr, NULL);
        error = "";
    }
    while(false);
    
    if (fp)
        fclose(fp);
    if(!error.empty())
    {
        std::cerr << error << std::endl;
        return false;
    }
    return true;
}

void setSymbol(unsigned char& red, unsigned char& green, unsigned char& blue, unsigned char code)
{
    unsigned char time = code;
    code &= 0x07;
    
    time = red & 0xFE;
    red = time + (int)(code & 0x1);
    time = green & 0xFE;
    green = time + (int)((code >> 1) & 0x1);

    time = blue & 0xFE;
    blue = time + (int)((code >> 2) & 0x1);
}

unsigned char getSymbol(unsigned char& red, unsigned char& green, unsigned char& blue)
{
    unsigned char code = 0;
    code |= (red & 0x1);

    code |= (green & 0x1) << 1;

    code |= (blue & 0x1) << 2;
    return code;
}

bool countIndexs(const ImageInfo& image, int& i, int& j, int& k)
{
    if (j == image.height)
        return false;
    
    if (i == image.width)
    {
        i = 0;
        k = 0;
        ++j;
    }
    return true;
}

bool countIndexsAndSymbol(const ImageInfo& image, int& i, int& j, int& k, int index, unsigned char& code)
{
    code = 0;
    for (size_t n = 0; n < 3; ++n, ++i)
    {
        if (!countIndexs(image, i, j, k))
            return false;
        unsigned char time = getSymbol(image.row_pointers[j][k], image.row_pointers[j][k + 1], image.row_pointers[j][k + 2]);
        k += 3 + index;
        time = time << n*3;
        code += time;
    }
    return true;
}


bool encoder(const char* filePathPng, const char* filePathNewPng, const char* filePathText)
{
    int size = 0;
    int i = 0, j = 0, k = 0;
    unsigned char code = '#'; //0x23
    
    std::ifstream file(filePathText);
    if(!file.is_open())
    {
        std::cerr << "File %s could not be opened for writing " << filePathText;
        return false;
    }

    file.seekg (0, file.end);
    int len = file.tellg();
    file.seekg (0, file.beg);

    std::string sizeText = std::to_string(len);
    std::string textFromFile(len + 1, '\0');
    file.read(&textFromFile[0], len);
    std::string textAll;
    textAll.push_back(code);
    textAll += sizeText;
    textAll.push_back(code);
    textAll += textFromFile; 
    ImageInfo image;
    read_png_file(filePathPng, &image);
    int index = 0;
    if (image.color_type == PNG_COLOR_TYPE_RGBA)
        ++index;
    
    while (size < textAll.size())
    {
        unsigned int time = textAll[size];
        for (size_t n = 0; n < 3; ++n, ++i)
        {
            if (!countIndexs(image, i, j, k))
                return false;
            setSymbol(image.row_pointers[j][k], image.row_pointers[j][k + 1], image.row_pointers[j][k + 2], time);
            k += 3 + index;
            time = time >> 3;
        }
        ++size;
    }
    write_png_file(filePathNewPng, &image);
    
    return true;
}


bool decoder(const char* filePathPng, std::string& text)
{
    int i = 0, j = 0;
    std::string sizeText;
    unsigned char code = 0;
    int k = 0;
    
    ImageInfo image;
    read_png_file(filePathPng, &image);
    int index = 0;
    if (image.color_type == PNG_COLOR_TYPE_RGBA)
        ++index;
    countIndexsAndSymbol(image, i, j, k, index, code);
    
    if (code != '#')
        return false;
    code = 0;
    while (code != '#')
    {
        if (!countIndexsAndSymbol(image, i, j, k, index, code))
            return false;
        if (code != '#')
            sizeText.push_back(code);
    }
    int length = std::stoi(sizeText);

    for (int s = 0; s < length; ++s)
    {
        if (!countIndexsAndSymbol(image, i, j, k, index, code))
            return false;
        text.push_back(code);
    }
    return true;
}

bool chi_square(const char* filePathPng, const char* filePathNewPng)
{
    int count = 0;
    ImageInfo image;
    read_png_file(filePathPng, &image);

    int index = 0;
    if (image.color_type == PNG_COLOR_TYPE_RGBA)
        ++index; 
        
    for (int j = 0; j < image.height; ++j)
    {
        int n = 0;
        std::vector<unsigned int> expected;
        std::vector<unsigned int> observed;
        std::vector<unsigned int> colorVector(256);
        for (int i = 0; i < image.width; ++i, n += 3 + index)
            colorVector[image.row_pointers[j][n + 1]] += 1;

        for (int k = 0; k <= 127; ++k)
        {
            expected.push_back(((colorVector[2 * k] + colorVector[2 * k + 1]) / 2));
            observed.push_back(colorVector[2 * k]);

        }
        unsigned int Sum = 0;
        for (int k = 0; k <= 127; ++k)
        {
            if (expected[k] == 0)
                continue;
            Sum += (((observed[k] - expected[k])*(observed[k] - expected[k])) / expected[k]);
        }
        n = 0;
        if (Sum > 80)
        {
            for (int i = 0; i < image.width; ++i, n += 3 + index)
            {
                image.row_pointers[j][n] = 255;
                image.row_pointers[j][n + 1] = image.row_pointers[j][n + 2] = 0;
            }
            ++count;
        }
    }
   
    std::cout << ((float)count / image.height) * 100 << "%" << std::endl;
    write_png_file(filePathNewPng, &image);

    return true;
}

int main()
{
    colorTest("../resources/image1.png", "../resources/imageColorBeforeEncoder.png");
    encoder("../resources/image1.png", "../resources/imageAfterEncoder.png", "../resources/text.txt");
    colorTest("../resources/imageAfterEncoder.png", "../resources/imageColorAfterEncoder.png");
    std::string text;
    decoder("../resources/imageAfterEncoder.png", text);
    std::cout << text << std::endl;
    chi_square("../resources/imageAfterEncoder.png", "../resources/chiSquare.png");
    return 0;
}
