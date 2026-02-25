// #include "eeg_dummy.h"
// #include <math.h>

// #define FS 250.0f
// #define PI 3.1415926f

// static float t = 0.0f;

// void eegDummyGenerate16(int16_t *ch)
// {
//     t += 1.0f / FS;

//     float alpha = sinf(2 * PI * 10 * t);   // 10 Hz, range: -1 to +1
//     float beta  = sinf(2 * PI * 20 * t);   // 20 Hz, range: -1 to +1
//     float theta = sinf(2 * PI * 6 * t);    // 6 Hz, range: -1 to +1

//     for (int i = 0; i < 16; i++)
//     {
//         // Noise kecil antara -0.1 sampai +0.1
//         float noise = ((rand() % 100) - 50) / 500.0f;
        
//         // Gabungkan sinyal (range total sekitar -2.0 sampai +2.0)
//         float sig = alpha + 0.6f * beta + 0.3f * theta + noise;
        
//         // OPSI 1: Range 0 sampai 800
//         // Normalisasi dari [-2, +2] ke [0, 800]
//         // ch[i] = (int16_t)((sig + 2.0f) * 200.0f);  // (sig+2)/4 * 800
        
//         // OPSI 2: Range -400 sampai +400
//         // ch[i] = (int16_t)(sig * 200.0f);
        
//         // OPSI 3: Range -800 sampai +800
//         ch[i] = (int16_t)(sig * 400.0f);
//     }
// }