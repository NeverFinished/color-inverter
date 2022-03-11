// SOURCE https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void rgb2hsv3(double in_r, double in_g, double in_b, double* hsv3)
{
    double      min, max, delta;
    double out_h, out_s, out_v; 

    min = in_r < in_g ? in_r : in_g;
    min = min  < in_b ? min  : in_b;

    max = in_r > in_g ? in_r : in_g;
    max = max  > in_b ? max  : in_b;

    out_v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out_s = 0;
        out_h = 0; // undefined, maybe nan?
        return;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out_s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out_s = 0.0;
        out_h = NAN;                            // its now undefined
        return;
    }
    if( in_r >= max )                           // > is bogus, just keeps compilor happy
        out_h = ( in_g - in_b ) / delta;        // between yellow & magenta
    else
    if( in_g >= max )
        out_h = 2.0 + ( in_b - in_r ) / delta;  // between cyan & yellow
    else
        out_h = 4.0 + ( in_r - in_g ) / delta;  // between magenta & cyan

    out_h *= 60.0;                              // degrees

    if( out_h < 0.0 )
        out_h += 360.0;

    hsv3[0] = out_h;
    hsv3[1] = out_s;
    hsv3[2] = out_v;
}

uint32_t hsv2rgb3(float h, float s, float v)
{
    double      hh, p, q, t, ff;
    long        i;
    double r, g, b;

    hh = h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch(i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;

    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
    default:
        r = v;
        g = p;
        b = q;
        break;
    }
    int ro = int(255.0 * r);
    int go = int(255.0 * g);
    int bo = int(255.0 * b);
    
    return ((uint32_t)ro << 16) | ((uint16_t)go << 8) | bo;  
}
