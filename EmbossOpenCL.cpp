#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>

#include "utils.h"  // read_kernel

#include <opencv2/opencv.hpp>

#include <CL/opencl.hpp> 

int main()
{
    // 1. Load the Image using OpenCV
    cv::Mat inputImage = cv::imread("input.jpg", cv::IMREAD_UNCHANGED);
    
    if (inputImage.empty()) {
        std::cout << "Could not load image! Check path.\n";
        return -1;
    }

    int width = inputImage.cols;
    int height = inputImage.rows;
    int channels = inputImage.channels();
    
    // Calculate total size in bytes
    size_t dataSize = width * height * channels * sizeof(unsigned char);

    std::cout << "Processing Image: " << width << "x" << height << " (" << channels << " channels)\n";

    // -------------------------------------------------------------------------
    // OPENCL SETUP
    // -------------------------------------------------------------------------

    // Get Platform
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.empty()) { std::cout << "No platforms found.\n"; return 1; }
    cl::Platform default_platform = all_platforms[0];  // take first available
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    // Get Device
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
    if (all_devices.empty()) { std::cout << "No devices found.\n"; return 1; }
    cl::Device default_device = all_devices[0];  // first available (sould be NVIDIA 3060)
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    // Create Context & Queue
    cl::Context context({ default_device });
    cl::CommandQueue queue(context, default_device);

    // -------------------------------------------------------------------------
    // Kernel Setup
    // -------------------------------------------------------------------------

    //std::string kernelSource = read_kernel("grayscale_kernel.cl");
    //std::string kernelSource = read_kernel("emboss_kernel.cl");
    std::string kernelSource = read_kernel("gray_and_emboss_2_kernel.cl");
    
    std::vector<std::string> sources;
    sources.push_back(kernelSource);

    cl::Program program(context, sources);
    if (program.build({ default_device }) != CL_SUCCESS) {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        return 1;
    }

    // Create Kernel
    cl::Kernel imageKernel(program, "processImage");

    // -------------------------------------------------------------------------
    // MEMORY ALLOCATION
    // -------------------------------------------------------------------------

    // Create Buffers on GPU (Input and Output)
    cl::Buffer dev_input(context, CL_MEM_READ_ONLY, dataSize);
    cl::Buffer dev_output(context, CL_MEM_WRITE_ONLY, dataSize);

    // Copy Image Data: Host (CPU) -> Device (GPU)
    queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, dataSize, inputImage.data);

    // -------------------------------------------------------------------------
    // KERNEL Arguments
    // -------------------------------------------------------------------------

    // Set Arguments
    imageKernel.setArg(0, dev_input);
    imageKernel.setArg(1, dev_output);
    imageKernel.setArg(2, width);
    imageKernel.setArg(3, height);
    imageKernel.setArg(4, channels);

    // -------------------------------------------------------------------------
    // Thread and Group Sizes
    // -------------------------------------------------------------------------

    // Define Grid Size (NDRange)
    // 1. Global Size: The total number of threads (Width x Height)
    cl::NDRange globalSize(width, height);
    
    // 2. Local Size: Let the driver decide automatically (16x16 probably idk)
    cl::NDRange localSize = cl::NullRange; 

    // -------------------------------------------------------------------------
	// ACTUAL KERNEL LAUNCH
    // -------------------------------------------------------------------------
    auto start_time = std::chrono::high_resolution_clock::now();  // starting timer
    queue.enqueueNDRangeKernel(imageKernel, cl::NullRange, globalSize, localSize);
    queue.finish();  // sync with gpu / wait to finish
    auto end_time = std::chrono::high_resolution_clock::now();
    double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    time_taken *= 1e-9; // Convert nanoseconds to seconds

    // printing timer
    std::cout << "Time taken: " << std::fixed << std::setprecision(6) << time_taken << " seconds\n";

    // -------------------------------------------------------------------------
    // GET RESULkS
    // -------------------------------------------------------------------------

    // Create a OpenCV container for the result
	cv::Mat outputImage = cv::Mat::zeros(height, width, inputImage.type()); // same type and size as input

    // Copy Data Back: Device (GPU) -> Host (CPU)
    queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, dataSize, outputImage.data);

    // save image
	cv::imwrite("output.jpg", outputImage);

    //// Show result
    cv::imshow("Input", inputImage);
    cv::imshow("Output (Swapped Channels)", outputImage);
    cv::waitKey(0);

    return 0;
}