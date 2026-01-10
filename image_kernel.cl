__kernel void processImage(__global const unsigned char* inputImage, 
                           __global unsigned char* outputImage, 
                           int width, 
                           int height, 
                           int channels) 
{
    // In OpenCL, we can get X and Y directly because we will launch a 2D Grid
    int x = get_global_id(0); // Corresponds to columns
    int y = get_global_id(1); // Corresponds to rows

    // Check bounds (just like the CUDA example)
    if (x >= width || y >= height) {
        return;
    }

    // Calculate the linear index (flattening 2D coordinates to 1D memory)
    // "width * channels" is the length of one full row in bytes
    int loc = (y * width * channels) + (x * channels);

    // Swap Red and Blue (BGR -> RGB or vice versa)
    // Note: We use the "loc" index to jump to the right pixel in the big array
    outputImage[loc + 0] = inputImage[loc + 2]; // Move R to B
    outputImage[loc + 1] = inputImage[loc + 1]; // Keep G same
    outputImage[loc + 2] = inputImage[loc + 0]; // Move B to R
    
    // If there is an alpha channel (4 channels), copy it too
    if (channels == 4) {
        outputImage[loc + 3] = inputImage[loc + 3];
    }
}
