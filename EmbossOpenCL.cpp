#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include "utils.h"

#include <opencv2/opencv.hpp>

#include <CL/opencl.hpp> 

int main()
{
    // 1. Load the Image using OpenCV
    // Make sure "input.jpg" is in your project folder!
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
    // OPENCL SETUP (Standard Boilerplate)
    // -------------------------------------------------------------------------

    // Get Platform
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.empty()) { std::cout << "No platforms found.\n"; return 1; }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    // Get Device
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
    if (all_devices.empty()) { std::cout << "No devices found.\n"; return 1; }
    cl::Device default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    // Create Context & Queue
    cl::Context context({ default_device });
    cl::CommandQueue queue(context, default_device);

    // Build Program
    //std::string kernelSource = read_kernel("grayscale_kernel.cl");
    //std::string kernelSource = read_kernel("emboss_kernel.cl");
    std::string kernelSource = read_kernel("gray_and_emboss_kernel.cl");
    
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
    // KERNEL EXECUTION
    // -------------------------------------------------------------------------

    // Set Arguments
    imageKernel.setArg(0, dev_input);
    imageKernel.setArg(1, dev_output);
    imageKernel.setArg(2, width);
    imageKernel.setArg(3, height);
    imageKernel.setArg(4, channels);

    // Define Grid Size (NDRange)
    // 1. Global Size: The total number of threads (Width x Height)
    cl::NDRange globalSize(width, height);
    
    // 2. Local Size: The size of the work-group (e.g., 16x16 block like in CUDA)
    // Note: We leave this Null usually, letting the driver decide, but to match CUDA 16x16:
    // cl::NDRange localSize(16, 16); 
    // Ideally, we pad globalSize to be a multiple of 16 if we force localSize.
    // For simplicity here, we let OpenCL decide the local size automatically:
    cl::NDRange localSize = cl::NullRange; 

    // Execute the kernel
    queue.enqueueNDRangeKernel(imageKernel, cl::NullRange, globalSize, localSize);

    // -------------------------------------------------------------------------
    // GET RESULTS
    // -------------------------------------------------------------------------

    // Create a container for the result
    cv::Mat outputImage = cv::Mat::zeros(height, width, inputImage.type());

    // Copy Data Back: Device (GPU) -> Host (CPU)
    queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, dataSize, outputImage.data);

    // save image
	cv::imwrite("output.jpg", outputImage);

    // Show result
    cv::imshow("Input", inputImage);
    cv::imshow("Output (Swapped Channels)", outputImage);
    cv::waitKey(0);

    return 0;
}