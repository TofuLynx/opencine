// Copyright (c) 2017 apertus° Association & contributors
// Project: OpenCine / OCcore
// License: GNU GPL Version 3 (https://www.gnu.org/licenses/gpl-3.0.en.html)

#ifndef RAWLOADER_H
#define RAWLOADER_H

#include <functional>
#include <unordered_map>

#include "OCImage.h"
#include "IImageLoader.h"

#include "OCcore_export.h"

#include "Memory/StaticAllocator.h"

namespace OC
{
    namespace DataProvider
    {
        class OCCORE_EXPORT RAWLoader : public IImageLoader
        {
            IAllocator* _allocator;

        public:
            RAWLoader();

            void Load(unsigned char *data, unsigned size, OCImage &image, IAllocator &allocator) override;
        };
    }
}

#endif //RAWLOADER_H
