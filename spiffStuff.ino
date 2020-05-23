/*
 *  LittleFStuff 
 */

//------------------------------------------------------------------------
bool readFileById(const char* fName, uint8_t mId)
{
  String rTmp;

  DebugTf("read [%s] ", fName);
  
  if (!LittleFS.exists(fName)) 
  {
    Debugln("Does not exist!");
    return false;
  }

  File f = LittleFS.open(fName, "r");

  while(f.available()) 
  {
    yield();
//  rTmp = f.readStringUntil('\n');
//  rTmp.replace("\r", "");
  }
  f.close();

  Debugln("Done!");
  return true;
  
} // readFileById()

//------------------------------------------------------------------------
bool writeFileById(const char* fName, uint8_t mId, const char *data)
{
  String rTmp;

  DebugTf("write [%s] ", fName);

  File file = LittleFS.open(fName, "w");
  if (!file) 
  {
    Debugf("open(%s, 'w') FAILED!!! --> Bailout\r\n", fName);
    return false;
  }
  yield();

  Debugln(F("Start writing data .. \r"));
  file.println(data);
  file.close();

  return true;
  
} // writeFileById()
 
