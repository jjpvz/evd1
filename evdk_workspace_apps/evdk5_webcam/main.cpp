/*! ***************************************************************************
 *
 * \brief     Webcam example for the EVDK5 project
 * \file      main.cpp
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      January 2025
 *
 * \note      Successfully tested with the following software versions:
 *            - GCC 14.2.0 x86_64-w64-mingw32 (ucrt64)
 *            - OpenCV 4.10.0 - build from sources with GCC 14.2.0
 *              x86_64-w64-mingw32 (ucrt64)
 *            - CMake for Visual Studio Code 0.0.17
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include "opencv2/opencv.hpp"

// If this application does not compile or run, first check if the following
// paths are added to the system environment variable PATH:
// - C:\opencv-x.y.z\build\bin
// - C:\opencv-x.y.z\build\lib

int main(void)
{
    printf("EVDK5 Webcam - %s %s\n", __DATE__, __TIME__);

    // -------------------------------------------------------------------------
    // Try to open the EVDK5 uvc video capture device
    // -------------------------------------------------------------------------
    // 0 is most likely your internal webcam
    // 1 is most likely the EVDK5
    cv::VideoCapture cap(1);

    if (!cap.isOpened())
    {
        printf("No EVDK5 video stream\n");
        return 0;
    }

    printf("Video stream opened\n");

    // -------------------------------------------------------------------------
    // Local variables
    // -------------------------------------------------------------------------
    cv::Mat cam;
    cv::namedWindow("cam", cv::WINDOW_NORMAL | cv::WINDOW_FREERATIO);

    // -------------------------------------------------------------------------
    // Start main loop
    // -------------------------------------------------------------------------
    printf("Press s to save an image or any other key to quit\n");

    while(true)
    {
        // ---------------------------------------------------------------------
        // Capture camera image in BGR format
        // ---------------------------------------------------------------------
        cap.read(cam);

        if(cam.empty())
        {
            printf("Video stream probably stopped\n");
            break;
        }

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------


        // Empty


        // ---------------------------------------------------------------------
        // Show images
        // ---------------------------------------------------------------------
        cv::imshow("cam", cam);

        // ---------------------------------------------------------------------
        // Save the image if 's' pressed
        // Break from the main loop if any other key is pressed
        // ---------------------------------------------------------------------
        char c = (char)cv::waitKey(1);

        if(c == 's')
        {
            cv::imwrite("image.bmp", cam);
            printf("Saved image.bmp\n");
        }
        else if(c != -1)
        {
            printf("Key press detected\n");
            break;
        }
    }

    // -------------------------------------------------------------------------
    // Cleanup
    // -------------------------------------------------------------------------
    cv::destroyAllWindows();
    cap.release();

    printf("EVDK5 application ended\n");

    return 0;
}
