#include "timing.h"

struct override {
    int8_t  msgNr;
    float   newValue;
    bool    enabled;
}; 

struct override OverRides[] = { 
  {
    OT_MSGID_CH_SETPOINT, 40.0, false
  },
  {
    OT_MSGID_ROOM_SETPOINT, 20.0, false
  },
  {
    OT_MSGID_ROOM_TEMP, 20.0, false
  }
};

int nrOverRides = sizeof(OverRides)/sizeof(struct override);

struct binfo {
    int     status;             // 0
    float   modulation_level;   // 17
    float   ch_pres;            // 18
    float   Tboiler;            // 25
    float   Tdhw;               // 26
    float   Toutside;           // 27
    float   Tret;               // 28
    int8_t  DHWLow, DHWHigh;    // 48
    int8_t  CHLow, CHHigh;      // 49
} binfo;

struct bcmd {
    float   Tset;       // 1
    float   MaxRelMod;  // 14
    float   TrSet;      // 16
    float   Tr;         // 24
    float   TdhwSet;    // 56
} bcmd;

#define DumpF(txt) sprintf(msgTxt, "%20.20s %6.2f", txt, data.f88());
#define DumpI(txt) sprintf(msgTxt, "%20.20s %6d",   txt, data.u16());
void DumpX(char *str, OpenthermData &data)
{
    Debugf("[ %s HB -> ", str);

    // valueHB

    if ( data.valueHB & B00000001)
        Debug ("CH ");
    if ( data.valueHB & B00000010)
        Debug ("DHW ");
        
    // valueLB

    if ( data.valueLB)
        Debugf(" LB <- ");
    if ( data.valueLB & B00000010)
        Debug ("CH ");
    if ( data.valueLB & B00000100)
        Debug ("DHW ");
    if ( data.valueLB & B00001000)
        Debug ("FLM ");

    Debug("]");
}

void printToDebug(int mode, OpenthermData &data) 
{
  char msgTxt[60];
  msgTxt[0]='\0';

  if (mode == MODE_LISTEN_MASTER)
  {
    DebugT("LM: ");
  } else {
    DebugT("LS: ");
  }
  
  if (data.type == OT_MSGTYPE_READ_DATA) {
    Debug("ReadData ");
  }
  else if (data.type == OT_MSGTYPE_READ_ACK) {
    Debug("ReadAck  ");
  }
  else if (data.type == OT_MSGTYPE_WRITE_DATA) {
    Debug("WriteData");
  }
  else if (data.type == OT_MSGTYPE_WRITE_ACK) {
    Debug("WriteAck ");
  }
  else if (data.type == OT_MSGTYPE_INVALID_DATA) {
    Debug("Inv.Data ");
  }
  else if (data.type == OT_MSGTYPE_DATA_INVALID) {
    Debug("DataInv. ");
  }
  else if (data.type == OT_MSGTYPE_UNKNOWN_DATAID) {
    Debug("Unknw.Id ");
  }
  else {
    Debug(data.type, BIN);
  }
  
  Debugf(" %03d %02x %02x", data.id, data.valueHB, data.valueLB);
  
  switch (data.id) {
    case 0:     DumpX("Status", data);
                break;
    case 1:     DumpF("Tset");
                break;
    case 16:    DumpF("Trset");
                break;
    case 17:    DumpF("RelMod");
                break;
    case 18:    DumpF("CH-pres");
                break;
    case 24:    DumpF("Tr");
                break;
    case 25:    DumpF("Tboiler");
                break;
    case 28:    DumpF("Tret");
                break;
    case 116:   DumpI("Burner Starts");
                break;
    case 120:   DumpI("Burner ops hrs");
                break;
              
  }
  Debugln(msgTxt);
}

void updateInfo(int mode, OpenthermData &data)
{
    // update info record based on response on read request

    // data read from boiler

    if (data.type == OT_MSGTYPE_READ_ACK)
    {
        switch (data.id) {
        case 0:     binfo.status = data.valueLB;
                    break;
        case 5:     // error flag
                    break;
        case 17:    binfo.modulation_level= data.f88();
                    break;
        case 18:    binfo.ch_pres = data.f88();
                    break;
        case 25:    binfo.Tboiler = data.f88();
                    break;
        case 26:    binfo.Tdhw = data.f88();
                    break;
        case 27:    binfo.Toutside = data.f88();
                    break;
        case 28:    binfo.Tret = data.f88();
                    break;
        case 48:    binfo.DHWLow = data.valueHB;
                    binfo.DHWHigh = data.valueLB;
                    break;
        case 49:    binfo.CHLow = data.valueHB;
                    binfo.CHHigh = data.valueLB;
                    break;
        default:    Debugf("Not caputed READ_ACK for %d\n", data.id);
        }
    }
  
    // data written form thermostate to boiler

    if (data.type == OT_MSGTYPE_WRITE_DATA)
    {
        switch (data.id) {
        case 1:     bcmd.Tset = data.f88();
                    break;
        case 2:     // config
                    break;
        case 14:    bcmd.MaxRelMod = data.f88();
                    break;
        case 16:    bcmd.TrSet = data.f88();
                    break;
        case 24:    bcmd.Tr = data.f88();
                    break;
        case 56:    bcmd.TdhwSet = data.f88();
                    break;
        default:    Debugf("Not caputed WRITE_DATA for %d\n", data.id);
        }
    }

}
void modifyMessage(int mode, OpenthermData &data)
{
    bool modified=false;

    if ( data.type == OT_MSGTYPE_WRITE_DATA )   // room thermostate sends info we might want to change
    {
        for (int o=0 ; o < nrOverRides ; o++)
        {
          Debugf("Override check %d\n", o);
          if ( OverRides[o].msgNr == data.id )
          {
            Debug("Override found: ");
            if( OverRides[o].enabled )
            {
              data.f88(OverRides[o].newValue);
              modified = true;
              Debugf("and changed to %.1f\n", OverRides[o].newValue);
            } else {
              Debugln("change disabled");
            }
            o = nrOverRides; // don't search further
          }
        }
    }

    if(modified) {
        Debugln(">> [MODIFIED - NEW VERSION BELOW] << ");
        printToDebug(mode, data);
    }
}

void myOpenthermLoop()
{
  if (mode == MODE_LISTEN_MASTER) {   // listen to room controller
 
    if (OPENTHERM::isSent() || OPENTHERM::isIdle() || OPENTHERM::isError()) {
      OPENTHERM::listen(THERMOSTAT_IN);
    } else if (OPENTHERM::getMessage(message)) {
      // printToDebug(mode,message);
      modifyMessage(mode, message);
      updateInfo(mode, message);

      OPENTHERM::send(BOILER_OUT, message); // forward message to boiler
      mode = MODE_LISTEN_SLAVE;
    }
  } else if (mode == MODE_LISTEN_SLAVE) { // listen to boiler
    if (OPENTHERM::isSent()) {
      OPENTHERM::listen(BOILER_IN, 800); // response need to be send back by boiler within 800ms
    }
    else if (OPENTHERM::getMessage(message)) {
      // printToDebug(mode, message);
      updateInfo(mode, message);

      OPENTHERM::send(THERMOSTAT_OUT, message); // send message back to thermostat
      mode = MODE_LISTEN_MASTER;
    }
    else if (OPENTHERM::isError()) {
      mode = MODE_LISTEN_MASTER;
      Debugln("<- Timeout");
    }
  }
 
}

// API part of the application

void sendStatus() 
{
  sendStartJsonObj("otinfo");

  sendJsonObj("status",           binfo.status);             // 0
  sendJsonObj("modulation_level", binfo.modulation_level);   // 17
  sendJsonObj("ch_pres",          binfo.ch_pres);            // 18
  sendJsonObj("Tboiler",          binfo.Tboiler);            // 25
  sendJsonObj("Tdhw",             binfo.Tdhw);               // 26
  sendJsonObj("Toutside",         binfo.Toutside);           // 27
  sendJsonObj("Tret",             binfo.Tret);               // 28
 
  sendJsonObj("Tset",             bcmd.Tset);       // 1  
  sendJsonObj("MaxRelMod",        bcmd.MaxRelMod);  // 14
  sendJsonObj("TrSet",            bcmd.TrSet);      // 16
  sendJsonObj("Tr",               bcmd.Tr);         // 24
  sendJsonObj("TdhwSet",          bcmd.TdhwSet);    // 56

  sendEndJsonObj();

}

