__kernel void processImage(__global const unsigned char* inputImage, 
                           __global unsigned char* outputImage, 
                           int width, 
                           int height, 
                           int channels) 
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    // Boundary check
    if (x >= width || y >= height) {
        return;
    }

    // Calculate the index for the start of this pixel (same as before)
    int loc = (y * width * channels) + (x * channels);

    // 1. Read the B, G, R values (Remember OpenCV is BGR)
    unsigned char blue  = inputImage[loc + 0];
    unsigned char green = inputImage[loc + 1];
    unsigned char red   = inputImage[loc + 2];

    // 2. Apply the Grayscale Formula
    // We cast to float for the math to preserve precision
    float grayValue = (0.21f * red) + (0.72f * green) + (0.07f * blue);
    // float grayValue = (0.33f * red) + (0.33f * green) + (0.33f * blue);

    // 3. Cast back to integer (clamping usually not needed here as sum <= 255)
    unsigned char result = (unsigned char)grayValue;

    // 4. Write the result to ALL channels
    // This keeps the output compatible with your 3-channel C++ host code
    outputImage[loc + 0] = result; // Blue channel
    outputImage[loc + 1] = result; // Green channel
    outputImage[loc + 2] = result; // Red channel

    // If there is an Alpha channel (transparency), keep it unchanged
    if (channels == 4) {
        outputImage[loc + 3] = inputImage[loc + 3];
    }
}
