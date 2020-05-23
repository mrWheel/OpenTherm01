#include "index.h"

void sendIndexPage()
{
  httpServer.send(200, "text/html", indexPage);

} // sendIndexPage()

// eof
