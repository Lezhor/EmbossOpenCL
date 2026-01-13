//#include <iostream>
//#include <vector>
//#include <chrono>
//#include <iomanip> // For std::setprecision
//
//#include <opencv2/opencv.hpp>
//
//// no OpenCL here
//
//int main()
//{
//    // 1. Load the Image
//    std::string filename = "input.jpg"; // same image as OpenCL version
//    cv::Mat inputImage = cv::imread(filename, cv::IMREAD_UNCHANGED);
//    
//    if (inputImage.empty()) {
//        std::cout << "Could not load image " << filename << "! Check path.\n";
//        return -1;
//    }
//
//    int width = inputImage.cols;
//    int height = inputImage.rows;
//    int channels = inputImage.channels();
//    
//    // Create an empty output image of the same size
//    cv::Mat outputImage = cv::Mat::zeros(height, width, inputImage.type());
//
//    // Pointers to raw data for faster access
//    unsigned char* inputPtr = inputImage.data;
//    unsigned char* outputPtr = outputImage.data;
//
//    std::cout << "Processing Image on CPU (Single Thread): " << width << "x" << height << "\n";
//
//    // -------------------------------------------------------------------------
//    // CPU CONVOLUTION LOGIC
//    // -------------------------------------------------------------------------
//
//    // Start Timer
//    auto start_time = std::chrono::high_resolution_clock::now();
//
//    // Emboss Kernel
//    const int filter[3][3] = {
//        {-1, -1,  0},
//        {-1,  0,  1},
//        { 0,  1,  1}
//    };
//
//    // NESTED LOOPS for traversing image and applying convolution
//	// Same logic as in gray_and_emboss_kernel.cl
//    for (int y = 0; y < height; y++) {
//        for (int x = 0; x < width; x++) {
//
//            // 1. Boundary Check
//            if (x < 1 || y < 1 || x >= width - 1 || y >= height - 1)
//                continue; 
//
//			float sumR = 0.0f;
//			float sumG = 0.0f;
//			float sumB = 0.0f;
//
//			// Iterate through neighbors
//			for (int ky = -1; ky <= 1; ky++) {
//				for (int kx = -1; kx <= 1; kx++) {
//					
//					int neighborX = x + kx;
//					int neighborY = y + ky;
//					int index = (neighborY * width * channels) + (neighborX * channels);
//
//					// Read individual channels
//					unsigned char b = inputPtr[index + 0];
//					unsigned char g = inputPtr[index + 1];
//					unsigned char r = inputPtr[index + 2];
//					
//					// Get the filter weight
//					int weight = filter[ky + 1][kx + 1];
//
//					// Accumulate independently
//					sumB += b * weight;
//					sumG += g * weight;
//					sumR += r * weight;
//				}
//			}
//
//			// --- SELECTION LOGIC ---
//			// Find which channel had the strongest reaction (biggest absolute value)
//			float absR = (sumR < 0) ? -sumR : sumR;
//			float absG = (sumG < 0) ? -sumG : sumG;
//			float absB = (sumB < 0) ? -sumB : sumB;
//
//			float winner = sumG; // Default to Green
//			float maxAbs = absG;
//
//			if (absR > maxAbs) {
//				winner = sumR;
//				maxAbs = absR;
//			}
//			if (absB > maxAbs) {
//				winner = sumB;
//				maxAbs = absB;
//			}
//
//			// Clamping & Bias
//			int finalVal = (int)(winner) + 128;
//            if (finalVal < 0) finalVal = 0;
//            if (finalVal > 255) finalVal = 255;
//
//            // 4. Write Result
//            int outIndex = (y * width * channels) + (x * channels);
//            outputPtr[outIndex + 0] = (unsigned char)finalVal;
//            outputPtr[outIndex + 1] = (unsigned char)finalVal;
//            outputPtr[outIndex + 2] = (unsigned char)finalVal;
//            
//            // Alpha channel copy (if needed)
//            if (channels == 4) {
//                outputPtr[outIndex + 3] = inputPtr[outIndex + 3];
//            }
//        }
//    }
//
//    // Stop Timer
//    auto end_time = std::chrono::high_resolution_clock::now();
//    double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
//    time_taken *= 1e-9; // Convert to seconds
//
//    std::cout << "CPU Time: " << std::fixed << std::setprecision(6) << time_taken << " seconds\n";
//
//    // Save result to verify it matches the GPU version
//    cv::imwrite("output_cpu_reference.jpg", outputImage);
//    
//    // Optional: Show image
//    // cv::imshow("CPU Output", outputImage);
//    // cv::waitKey(0);
//
//    return 0;
//}