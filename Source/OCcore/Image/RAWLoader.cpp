// Copyright (c) 2017 apertus° Association & contributors
// Project: OpenCine / OCcore
// License: GNU GPL Version 3 (https://www.gnu.org/licenses/gpl-3.0.en.html)

#include "RAWLoader.h"

#include <iostream>
#include <memory>

#include "Log/Logger.h"

#include "libraw/libraw.h"
#include "BayerFramePreProcessor.h"

using namespace OC::DataProvider;

RAWLoader::RAWLoader()
{
    _allocator = nullptr;
}

void RAWLoader::Load(unsigned char* data, unsigned size, OCImage& image, IAllocator& allocator)
{
    // Create Image Processor.
    std::unique_ptr<LibRaw> iProcessor(new LibRaw());

    // Open the file and load it.
    iProcessor->open_buffer(data, size);

    // Set image size.
    image.SetWidth (iProcessor->imgdata.sizes.iwidth);
    image.SetHeight(iProcessor->imgdata.sizes.iheight);

    // Unpack the image.
    iProcessor->unpack();

    // Get offset to first visible pixel.
    int offset = iProcessor->imgdata.sizes.raw_width*iProcessor->imgdata.sizes.top_margin + iProcessor->imgdata.sizes.left_margin;

    // Get image dimesions (Different from raw data dimensions).
    int imageSize = iProcessor->imgdata.sizes.iwidth * iProcessor->imgdata.sizes.iheight;

    // Determine Bayer Pattern.
    if (iProcessor->COLOR(0,0) == 0)
    {
        image.SetBayerPattern (BayerPattern::RGGB);
    }
    else if (iProcessor->COLOR(0,1) == 0)
    {
        image.SetBayerPattern (BayerPattern::GRBG);
    }
    else if (iProcessor->COLOR(1,0) == 0)
    {
        image.SetBayerPattern (BayerPattern::GBRG);
    }
    else if (iProcessor->COLOR(1,1) == 0)
    {
        image.SetBayerPattern (BayerPattern::BGGR);
    }

    _allocator = &allocator;

    std::unique_ptr<BayerFramePreProcessor> frameProcessor(new BayerFramePreProcessor());

    image.SetRedChannel(_allocator->Allocate(imageSize));
    image.SetGreenChannel(_allocator->Allocate(imageSize));
    image.SetBlueChannel(_allocator->Allocate(imageSize));

    frameProcessor->SetData(&(iProcessor->imgdata.rawdata.raw_image[offset]), image);

    frameProcessor->Process();

    image.SetRedChannel(frameProcessor->GetDataRed());
    image.SetGreenChannel(frameProcessor->GetDataGreen());
    image.SetBlueChannel(frameProcessor->GetDataBlue());

    iProcessor->recycle();
}
