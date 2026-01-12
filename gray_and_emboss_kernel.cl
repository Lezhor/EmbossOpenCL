__kernel void processImage(__global const unsigned char* inputImage, 
                           __global unsigned char* outputImage, 
                           int width, 
                           int height, 
                           int channels) 
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x < 1 || y < 1 || x >= width - 1 || y >= height - 1) return;

    // Emboss Filter
    const int filter[3][3] = {
        {-2, -1,  0},
        {-1,  1,  1},
        { 0,  1,  2}
    };

    // const int filter[3][3] = {
        // {-1, -1,  0},
        // {-1,  0,  1},
        // { 0,  1,  1}
    // };

    float sum = 0.0f;

    // Iterate through neighbors
    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            
            int neighborX = x + kx;
            int neighborY = y + ky;
            int index = (neighborY * width * channels) + (neighborX * channels); // neighbor location

            unsigned char b = inputImage[index + 0];
            unsigned char g = inputImage[index + 1];
            unsigned char r = inputImage[index + 2];

            float gray = (0.21f * r) + (0.72f * g) + (0.07f * b);

            sum += gray * filter[ky + 1][kx + 1];
        }
    }

    // Clamping & Bias
    int finalVal = (int)(sum) + 128;
    if (finalVal < 0) finalVal = 0;
    if (finalVal > 255) finalVal = 255;

    // Write Gray result to all 3 channels
    int outIndex = (y * width * channels) + (x * channels);
    outputImage[outIndex + 0] = (unsigned char)finalVal;
    outputImage[outIndex + 1] = (unsigned char)finalVal;
    outputImage[outIndex + 2] = (unsigned char)finalVal;
    
    // alpha
    if (channels == 4) outputImage[outIndex + 3] = inputImage[outIndex + 3];
}