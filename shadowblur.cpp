/*
    This file is a part of the KDE project

    Copyright © 2006 Zack Rusin <zack@kde.org>
    Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>

    The stack blur algorithm was invented by Mario Klingemann
    <mario@quasimondo.com>

    This implementation incorporates performance improvements from
    Anti-Grain Geometry Version 2.4,
    Copyright © Maxim Shemanarev (http://www.antigrain.com)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


static const quint32 stack_blur8_mul[255] =
{
    512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
    454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
    482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
    437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
    497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
    320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
    446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
    329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
    505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
    399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
    324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
    268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
    451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
    385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
    332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
    289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static const quint32 stack_blur8_shr[255] =
{
    9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};


inline static void blurHorizontal(QImage &image, int *stack, int div, int radius)
{
    int stackpointer;
    int stackstart;

    quint32 * const pixels = reinterpret_cast<quint32 *>(image.bits());
    quint32 pixel;

    int w = image.width();
    int h = image.height();
    int wm = w - 1;

    int mul_sum = stack_blur8_mul[radius];
    int shr_sum = stack_blur8_shr[radius];

    int sum, sum_in, sum_out;

    for (int y = 0; y < h; y++)
    {
        sum     = 0;
        sum_in  = 0;
        sum_out = 0;

        const int yw = y * w;
        pixel = pixels[yw];
        for (int i = 0; i <= radius; i++)
        {
            stack[i] = qAlpha(pixel);

            sum += stack[i] * (i + 1);
            sum_out += stack[i];
        }

        for (int i = 1; i <= radius; i++)
        {
            pixel = pixels[yw + qMin(i, wm)];

            int *stackpix = &stack[i + radius];
            *stackpix = qAlpha(pixel);

            sum    += *stackpix * (radius + 1 - i);
            sum_in += *stackpix;
        }

        stackpointer = radius;
        for (int x = 0, i = yw; x < w; x++)
        {
            pixels[i++] = (((sum * mul_sum) >> shr_sum) << 24) & 0xff000000;

            sum -= sum_out;

            stackstart = stackpointer + div - radius;
            if (stackstart >= div)
                stackstart -= div;

            int *stackpix = &stack[stackstart];

            sum_out -= *stackpix;

            pixel = pixels[yw + qMin(x + radius + 1, wm)];

            *stackpix = qAlpha(pixel);

            sum_in += *stackpix;
            sum    += sum_in;

            if (++stackpointer >= div)
                stackpointer = 0;

            stackpix = &stack[stackpointer];

            sum_out += *stackpix;
            sum_in  -= *stackpix;
        } // for (x = 0, ...)
    } // for (y = 0, ...)
}


inline static void blurVertical(QImage &image, int *stack, int div, int radius)
{
    quint32 * const pixels = reinterpret_cast<quint32 *>(image.bits());

    int w = image.width();
    int h = image.height();
    int hm = h - 1;

    int mul_sum = stack_blur8_mul[radius];
    int shr_sum = stack_blur8_shr[radius];

    int sum, sum_in, sum_out;

    int stackpointer;
    int stackstart;

    quint32 pixel;

    for (int x = 0; x < w; x++)
    {
        sum     = 0;
        sum_in  = 0;
        sum_out = 0;

        pixel = pixels[x];
        for (int i = 0; i <= radius; i++)
        {
            stack[i] = qAlpha(pixel);

            sum += stack[i] * (i + 1);
            sum_out += stack[i];
        }

        for (int i = 1; i <= radius; i++)
        {
            pixel = pixels[qMin(i, hm) * w + x];

            int *stackpix = &stack[i + radius];
            *stackpix = qAlpha(pixel);

            sum    += *stackpix * (radius + 1 - i);
            sum_in += *stackpix;
        }

        stackpointer = radius;
        for (int y = 0, i = x; y < h; y++, i += w)
        {
            pixels[i] = (((sum * mul_sum) >> shr_sum) << 24) & 0xff000000;

            sum -= sum_out;

            stackstart = stackpointer + div - radius;
            if (stackstart >= div)
                stackstart -= div;

            int *stackpix = &stack[stackstart];

            sum_out -= *stackpix;

            pixel = pixels[qMin(y + radius + 1, hm) * w + x];

            *stackpix = qAlpha(pixel);

            sum_in += *stackpix;
            sum    += sum_in;

            if (++stackpointer >= div)
                stackpointer = 0;

            stackpix = &stack[stackpointer];

            sum_out += *stackpix;
            sum_in  -= *stackpix;
        } // for (y = 0, ...)
    } // for (x = 0, ...)
}


void shadowBlur(QImage &image, int radius)
{
    if (radius < 1)
        return;

    int div = radius * 2 + 1;
    int *stack  = new int[div];

    blurHorizontal(image, stack, div, radius);
    blurVertical(image, stack, div, radius);

    delete [] stack;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
