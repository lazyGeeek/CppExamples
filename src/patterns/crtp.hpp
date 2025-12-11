#pragma once
#ifndef PATTERNS_CRTP_HPP_
#define PATTERNS_CRTP_HPP_

#include <chrono>
#include <iostream>

namespace Patterns
{
    // To store dimensions of an image
    class Dimension
    {
    public:
        Dimension(int x, int y)
        {
            m_x = x;
            m_y = y;
        }

    private:
        int m_x = 0;
        int m_y = 0;
    };

    // Base class for all image types
    class ImageInherit
    {
    public:
        virtual void Draw() = 0;
        virtual Dimension GetDimensionInPixels() = 0;

    protected:
        int m_dimensionX = 0;
        int m_dimensionY = 0;
    };

    // For Tiff Images
    class TiffImageInherit : public ImageInherit
    {
    public:
        void Draw() { }

        Dimension GetDimensionInPixels()
        {
            return Dimension(m_dimensionX, m_dimensionY);
        }
    };

    // Base class for all image types. The template
    // parameter T is used to know type of derived
    // class pointed by pointer.
    template <class T>
    class ImageCRTP
    {
    public:
        void Draw()
        {
            // Dispatch call to exact type
            static_cast<T*>(this)->Draw();
        }

        Dimension GetDimensionInPixels()
        {
            // Dispatch call to exact type
            static_cast<T*>(this)->GetDimensionInPixels();
        }

    protected:
        ImageCRTP() = default;
        friend T;

        int m_dimensionX = 0;
        int m_dimensionY = 0;
    };

    // For Tiff Images
    class TiffImageCRTP : public ImageCRTP<TiffImageCRTP>
    {
    public:
        void Draw() { }
        Dimension GetDimensionInPixels()
        {
            return Dimension(m_dimensionX, m_dimensionY);
        }
    };

    void TestCRTP()
    {
        // An image type
        ImageInherit* imageInherit = new TiffImageInherit;

        // Store time before virtual function calls
        auto then = std::chrono::high_resolution_clock::now();

        // Call Draw 1000 times to make sure performance
        // is visible
        for (int i = 0; i < 1000; ++i)
            imageInherit->Draw();

        // Store time after virtual function calls
        auto now = std::chrono::high_resolution_clock::now();

        std::cout << "Time taken for basic inheritance: "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count()
                  << " nanoseconds" << std::endl;

        // An image type
        ImageCRTP<TiffImageCRTP>* imageCRTP = new TiffImageCRTP;

        // Store time before virtual function calls
        then = std::chrono::high_resolution_clock::now();

        // Call Draw 1000 times to make sure performance
        // is visible
        for (int i = 0; i < 1000; ++i)
            imageCRTP->Draw();

        // Store time after virtual function calls
        now = std::chrono::high_resolution_clock::now();

        std::cout << "Time taken for CRTP: "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count()
                  << " nanoseconds" << std::endl;
    }
}

#endif // PATTERNS_CRTP_HPP_