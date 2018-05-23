// Copyright (c) 2018 apertus° Association & contributors
// Project: OpenCine / OCcore
// License: GNU GPL Version 3 (https://www.gnu.org/licenses/gpl-3.0.en.html)

#include "BilinearDebayer.h"

// TODO: Add multi-threading.
#include <thread>

#include <Log/Logger.h>

BilinearDebayer::BilinearDebayer(OCImage &image)
{
    _width = image.Width();
    _height = image.Height();
    _size = _width * _height;

    _redChannel = static_cast<uint16_t*>(image.RedChannel());
    _greenChannel = static_cast<uint16_t*>(image.GreenChannel());
    _blueChannel = static_cast<uint16_t*>(image.BlueChannel());

    _pattern = image.GetBayerPattern();
    SetPatternOffsets(_pattern);

    OC_LOG_INFO("\nConsidering width as " + std::to_string(_width) + ":\n" + std::to_string(_patternOffsets[0]) + "\n" + std::to_string(_patternOffsets[1]) + "\n" + std::to_string(_patternOffsets[2]) + "\n" + std::to_string(_patternOffsets[3]) + "\n");
}

BilinearDebayer::~BilinearDebayer()
{
}

void BilinearDebayer::DebayerBottomRight(uint16_t *channel)
{
    for(int index = _patternOffsets[0]; index < _size; index += 2)
    {
        channel[index] = ( channel[index - _width - 1] + channel[index - _width + 1] + channel[index + _width - 1] + channel[index + _width + 1] ) >> 2;
        channel[index + 1] = ( channel[index + _width + 1] + channel[index - _width + 1] ) >> 1;
        channel[index + _width] = ( channel[index + _width - 1] + channel[index + _width + 1] ) >> 1;
        if ((index + 3) % _width <= 1)
            index += _width + 2;
    }
}

void BilinearDebayer::DebayerBottomLeft(uint16_t *channel)
{
    for(int index = _patternOffsets[0]; index < _size; index += 2)
    {
        channel[index] = ( channel[index - _width - 1] + channel[index - _width + 1] + channel[index + _width - 1] + channel[index + _width + 1] ) >> 2;
        channel[index - 1] = ( channel[index + _width - 1] + channel[index - _width - 1] ) >> 1;
        channel[index + _width] = ( channel[index + _width - 1] + channel[index + _width + 1] ) >> 1;
        if ((index + 3) % _width <= 1)
            index += _width + 2;
    }
}

void BilinearDebayer::DebayerGreen()
{
    for(int index = _patternOffsets[1]; index < _size; index += 2)
    {
        _greenChannel[index] = ( _greenChannel[index - _width] + _greenChannel[index - 1] + _greenChannel[index + 1] + _greenChannel[index + _width] ) >> 2;
        if ((index + 3) % _width <= 1)
            index += _width + 2;
    }
    for(int index = _patternOffsets[2]; index < _size; index += 2)
    {
         _greenChannel[index] = ( _greenChannel[index - _width] + _greenChannel[index - 1] + _greenChannel[index + 1] + _greenChannel[index + _width] ) >> 2;
        if ((index + 3) % _width <= 1)
            index += _width + 2;
    }
}

void BilinearDebayer::DebayerTopLeft(uint16_t *channel)
{
    for(int index = _patternOffsets[3]; index < _size; index += 2)
    {
        channel[index] = ( channel[index - _width - 1] + channel[index - _width + 1] + channel[index + _width - 1] + channel[index + _width + 1] ) >> 2;
        channel[index - 1] = ( channel[index + _width - 1] + channel[index - _width - 1] ) >> 1;
        channel[index - _width] = ( channel[index - _width - 1] + channel[index - _width + 1] ) >> 1;
        if ((index + 3) % _width <= 1)
            index += _width + 2;
    }
}

void BilinearDebayer::DebayerTopRight(uint16_t *channel)
{
    for(int index = _patternOffsets[3]; index < _size; index += 2)
    {
        channel[index] = ( channel[index - _width - 1] + channel[index - _width + 1] + channel[index + _width - 1] + channel[index + _width + 1] ) >> 2;
        channel[index + 1] = ( channel[index + _width + 1] + channel[index - _width + 1] ) >> 1;
        channel[index - _width] = ( channel[index - _width - 1] + channel[index - _width + 1] ) >> 1;
        if ((index + 3) % _width <= 1)
            index += _width + 2;
    }
}

void BilinearDebayer::DemosaicBorders(uint16_t *channel)
{
    int size = _size - _width;
    for(int index = 0; index < _width; index += 2)
    {
        channel[index] = channel[index + _width];
        channel[index + 1] = channel[index + _width + 1];
        channel[size + index] = channel[size + index - _width];
        channel[size + index + 1] = channel[size + index - _width + 1];
    }
    for(int index = 0; index < _height; index += 2)
    {
        channel[(index * _width)] = channel[(index * _width) + 1];
        channel[(index + 1) * _width] = channel[((index + 1) * _width) + 1];
        channel[((index + 1) * _width) - 1] = channel[((index + 1) * _width) - 2];
        channel[((index + 2) * _width) - 1] = channel[((index + 2) * _width) - 2];
    }
}

void BilinearDebayer::Process()
{
    switch (_pattern) {
    case BayerPattern::RGGB:
        BilinearDebayer::DebayerBottomRight(_redChannel);
        BilinearDebayer::DebayerTopLeft(_blueChannel);
        break;
    case BayerPattern::BGGR:
        BilinearDebayer::DebayerBottomRight(_blueChannel);
        BilinearDebayer::DebayerTopLeft(_redChannel);
        break;
    case BayerPattern::GRBG:
        BilinearDebayer::DebayerBottomLeft(_redChannel);
        BilinearDebayer::DebayerTopRight(_blueChannel);
        break;
    case BayerPattern::GBRG:
        BilinearDebayer::DebayerBottomLeft(_blueChannel);
        BilinearDebayer::DebayerTopRight(_redChannel);
        break;
    default:
        break;
    }
    BilinearDebayer::DebayerGreen();
    BilinearDebayer::DemosaicBorders(_blueChannel);
    BilinearDebayer::DemosaicBorders(_greenChannel);
    BilinearDebayer::DemosaicBorders(_redChannel);
}

void BilinearDebayer::DebayerNearest(int red, int green0, int green1, int blue)
{
    for(int index = 0; index < _size; index += 2)
    {
        _redChannel[index + green0] = _redChannel[index + red];
        _redChannel[index + green1] = _redChannel[index + red];
        _redChannel[index + blue]   = _redChannel[index + red];

        _greenChannel[index + red]  = _greenChannel[index + green0];
        _greenChannel[index + blue] = _greenChannel[index + green1];

        _blueChannel[index + red]    = _blueChannel[index + blue];
        _blueChannel[index + green0] = _blueChannel[index + blue];
        _blueChannel[index + green1] = _blueChannel[index + blue];

        if ( ((index + 2) % _width) == 0 ) {
            index += _width;
        }
    }
}

void BilinearDebayer::ProcessNearest()
{
    // Nearest Interpolation Processor.
    switch (_pattern) {
    case BayerPattern::RGGB:
        DebayerNearest(0, 1, _width, _width + 1);
        break;
    case BayerPattern::BGGR:
        DebayerNearest(_width + 1, 1, _width, 0);
        break;
    case BayerPattern::GRBG:
        DebayerNearest(1, 0, _width + 1, _width);
        break;
    case BayerPattern::GBRG:
        DebayerNearest(_width, 0, _width + 1, 1);
        break;
    default:
        break;
    }
}

void BilinearDebayer::SetPatternOffsets(BayerPattern pattern)
{
    switch (pattern) {
    case BayerPattern::RGGB:
    case BayerPattern::BGGR:
        _patternOffsets[0] = _width + 1;
        _patternOffsets[1] = _width + 1;
        _patternOffsets[2] = (2 * _width) + 2;
        _patternOffsets[3] = (2 * _width) + 2;
        break;
    case BayerPattern::GRBG:
    case BayerPattern::GBRG:
        _patternOffsets[0] = _width + 2;
        _patternOffsets[1] = _width + 2;
        _patternOffsets[2] = (2 * _width) + 1;
        _patternOffsets[3] = (2 * _width) + 1;
        break;
    default:
        break;

    }
}
