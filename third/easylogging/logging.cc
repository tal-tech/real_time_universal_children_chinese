#include "logging.h"

#include "boost/thread.hpp"

namespace base
{

boost::mutex g_mutex;

void LoggerInitOnce() {
  boost::lock_guard<boost::mutex> lock(g_mutex);

  static bool init = false;
  if (!init) {
    el::Configurations conf;
    conf.setToDefault();
#if (defined(MS_DEBUG_VIEW_OPEN) || !defined(NDEBUG))
    conf.parseFromText("*GLOBAL:\n\
                        FORMAT = [%datetime][%level][%thread][%func][%line]----%msg----\n\
                        ENABLED = true\n\
                        TO_FILE = true\n\
                        Filename = \"vadec.log\"\n\
                        TO_STANDARD_OUTPUT = true");
#else
    conf.parseFromText("*GLOBAL:\n\
                        ENABLE = false\n\
                        TO_FILE = false\n\
                        TO_STANDARD_OUTPUT = false");
#endif
    el::Loggers::reconfigureAllLoggers(conf);
    init = true;
  }
}

}
