#pragma once

#ifdef _WIN32
#   include <thread>
#   include <memory>
#   include <condition_variable>
#   include <vector>
#   include <string>
#   include <list>
#   include <map>
#   include <algorithm>
#   include <system_error>
#   include <iostream>

    using std::mutex;
    using std::lock_guard;
    using std::condition_variable;
    using std::thread;
    using std::unique_ptr;
    using std::vector;
    using std::string;

#endif //  _WIN32
#pragma warning(disable : 4351)
