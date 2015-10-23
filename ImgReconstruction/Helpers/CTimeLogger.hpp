//
//  CTimeLogger.hpp
//  ImgReconstruction
//
//  Created by Alexander Bochkarev on 22.10.15.
//  Copyright © 2015 Alexander Bochkarev. All rights reserved.
//

#include <time.h>

class CTimeLogger
{
public:
    static void StartLogging();
    static void Print(const std::string& decr);
private:
    static clock_t _time;
};
