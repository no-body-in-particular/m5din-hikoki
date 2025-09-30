#include "M5DinMeter.h"
#include <HikokiSerial.h>


HikokiSerial serial1(13, 2, 1);  // RX, TX

#define TURNOFF_SECONDS 20000  // seconds without input before the screen turns off


size_t split_to(uint8_t delim, uint8_t* src, size_t len, uint8_t** dest, size_t dest_count) {
  size_t prev_idx = 0;
  size_t str_count = 0;

  len = min(len, strlen((char*)src));

  for (size_t i = 0; i <= len && str_count < dest_count; i++) {
    if (src[i] == delim || src[i] == '\0' || i == len) {
      size_t cur_len = i - prev_idx;
      dest[str_count] = src + prev_idx;
      for (; cur_len > 0 && isspace(dest[str_count][0]) && dest[str_count] < &src[len];) {
        dest[str_count]++;
        cur_len--;
      }
      str_count++;
      if (src[i] == '\0' || i == len) {
        return str_count;
      }
      prev_idx = i + 1;
      src[i] = 0;
    }
  }

  return str_count;
}

const __FlashStringHelper* batteryType(uint16_t typeCode) {
  switch (typeCode) {
    case 0x1:
      return F("Unconnectable");
    case 0x2:
      return F("BSL36A18");
    case 0x3:
      return F("BSL36B18");
    case 0x4:
      return F("BSL36200(MV Tool Adaptor)");
    case 0x5:
      return F("ET36A(AC/DC Adaptor)");
    case 0x6:
      return F("BSL1215");
    case 0x7:
      return F("BSL1240M");
    case 0x8:
      return F("BSL1225M");
    case 0x9:
      return F("BSL36C18");
    case 0xA:
      return F("BSL36A18B");
    case 0xB:
      return F("BSL36B18B");
    case 0xC:
      return F("ET36A(AC/DC Adaptor");
    case 0xD:
      return F("BSL1820M");
    case 0xE:
      return F("BSL1830M");
    case 0xF:
      return F("BSL1840M");
    case 0x10:
      return F("BSL1850MA");
    case 0x11:
      return F("BSL36A18X");
    case 0x12:
      return F("BSL36B18X");
    case 0x13:
      return F("BSL36A18BX");
    case 0x14:
      return F("BSL36B18BX");
    case 0x15:
      return F("BSL1220M");
    case 0x16:
      return F("BSL3640MVT");
    case 0x17:
      return F("BSL3640MVBT");
    case 0x18:
      return F("BSL1240MT");
    default:
      return F("Unknown");
  }
}

bool isBroken(uint16_t batteryCode, uint16_t errorFlags[]) {
  if (batteryCode == 0x2 || batteryCode == 0x3 || batteryCode == 0x9 || batteryCode == 0x11 || batteryCode == 0x12) {
    return (errorFlags[0] & 0x42 || errorFlags[2] & 0xE0 || errorFlags[3] & 0x80 || errorFlags[5] & 0x13 || errorFlags[6] & 0x3) > 0;
  }

  if (batteryCode == 0xA || batteryCode == 0xB || batteryCode == 0x17 || batteryCode == 0x14 || batteryCode == 0x13) {
    return (errorFlags[0] & 0x42 || errorFlags[2] & 0xC0 || errorFlags[5] & 0x33 || errorFlags[6] & 0x3) > 0;
  }

  if (batteryCode == 0xD || batteryCode == 0xE || batteryCode == 0xF || batteryCode == 0x10) {
    return (errorFlags[0] & 0x2 || errorFlags[3] & 0x80 || errorFlags[6] & 0x3) > 0;
  }

  return false;
}

const __FlashStringHelper* errorType(uint16_t typeCode) {
  switch (typeCode) {
    case 0x01:
      return F("Upper Side Over Charge");
    case 0x02:
      return F("Over Charge cell L1");
    case 0x03:
      return F("Over Charge cell L2");  //     return (errorCode==0x42 || errorCode==0xE0 || errorCode==0x80 || errorCode==0x13 || errorCode==0x3);
    case 0x04:
      return F("Over Charge cell L3");
    case 0x05:
      return F("Over Charge cell L4");
    case 0x06:
      return F("Over Charge cell L5");
    case 0x08:
      return F("Over Charge 2");
    case 0x09:
      return F("Over Charge 2(NOT Over Charge)");
    case 0x0B:
      return F("Charge overtemp Upper");
    case 0x0C:
      return F("Charge overtemp Lower");
    case 0x0E:
      return F("Partial Charge(Upper C)");
    case 0x0F:
      return F("Partial Charge(Lower C)");
    case 0x10:
      return F("Partial Charge(Upper -)");
    case 0x11:
      return F("Partial Charge(Lower -)");
    case 0x13:
      return F("Over Charge Current");
    case 0x15:
      return F("Abnormal Cell_Upper");
    case 0x16:
      return F("Abnormal Cell_L1");
    case 0x17:
      return F("Abnormal Cell_L2");
    case 0x18:
      return F("Abnormal Cell_L3");
    case 0x19:
      return F("Abnormal Cell_L4");
    case 0x1A:
      return F("Abnormal Cell_L5");
    case 0x1C:
      return F("Over Discharge 1_Upper");
    case 0x1D:
      return F("Over Discharge 1_L1");
    case 0x1E:
      return F("Over Discharge 1_L2");
    case 0x1F:
      return F("Over Discharge 1_L3");
    case 0x20:
      return F("Over Discharge 1_L4");
    case 0x21:
      return F("Over Discharge 1_L5");
    case 0x22:
      return F("Over Discharge 2_Upper");
    case 0x23:
      return F("Over Discharge 2_L1");
    case 0x24:
      return F("Over Discharge 2_L2");
    case 0x25:
      return F("Over Discharge 2_L3");
    case 0x26:
      return F("Over Discharge 2_L4");
    case 0x27:
      return F("Over Discharge 2_L5");
    case 0x28:
      return F("Over Discharge 3");
    case 0x29:
      return F("Over Discharge 4");
    case 0x2A:
      return F("Over Discharge 5");
    case 0x2B:
      return F("Over Discharge 6");
    case 0x33:
      return F("Overtemp Upper(Discharge)");
    case 0x34:
      return F("Overtemp Lower(Discharge)");
    case 0x35:
      return F("Upper temp 60 degrees");
    case 0x36:
      return F("Upper temp 65 degrees");
    case 0x37:
      return F("Upper temp 70 degrees");
    case 0x38:
      return F("Upper temp 75 degrees");
    case 0x39:
      return F("Lower temp 60 degrees");
    case 0x3A:
      return F("Lower temp 65 degrees");
    case 0x3B:
      return F("Lower temp 70 degrees");
    case 0x3C:
      return F("Lower temp 75 degrees");
    case 0x43:
      return F("Cell_TH Disconnect Upper");
    case 0x44:
      return F("Cell_TH Disconnect Lower");
    case 0x45:
      return F("Cell_TH Short(Upper Bank)");
    case 0x46:
      return F("Cell_TH Short(Lower Bank)");
    case 0x48:
      return F("Current Detect Error");
    case 0x49:
      return F("Cell Voltage Detect Error");
    case 0x4A:
      return F("VIN12 Voltage Detect Error");
    case 0x4B:
      return F("TH Resistor Detect Error");
    case 0x4C:
      return F("Upper Bank Voltage Detect Error");
    case 0x4E:
      return F("Bank Voltage Unbalance L > U");
    case 0x4F:
      return F("Bank Voltage Unbalance L < U");
    case 0x50:
      return F("Too Bad Balance");
    case 0x52:
      return F("Temperature Unbalance L > U");
    case 0x53:
      return F("Temperature Unbalance L < U");
    case 0x55:
      return F("Cell Unbalance");
    case 0x59:
      return F("Abnormally High Temperature");
    case 0x5A:
      return F("Extreme Overdischarge");
    case 0x65:
      return F("Over Charge 1(Upper)");
    case 0x66:
      return F("Over Charge 1(L1)");
    case 0x67:
      return F("Over Charge 1(L2)");
    case 0x68:
      return F("Over Charge 1(L3)");
    case 0x69:
      return F("Over Charge 1(L4)");
    case 0x6A:
      return F("Over Charge 1(L5)");
    case 0x6C:
      return F("Over Charge 2");
    case 0x6D:
      return F("Over Charge 2(NOT Over Charge 1)");
    case 0x80:
      return F("Over Discharge 1(Upper)");
    case 0x81:
      return F("Over Discharge 1(L1");
    case 0x82:
      return F("Over Discharge 1(L2)");
    case 0x83:
      return F("Over Discharge 1(L3)");
    case 0x84:
      return F("Over Discharge 1(L4)");
    case 0x85:
      return F("Over Discharge 1(L5)");
    case 0x86:
      return F("Over Discharge 2(Upper)");
    case 0x87:
      return F("Over Discharge 2(L1)");
    case 0x88:
      return F("Over Discharge 2(L2)");
    case 0x89:
      return F("Over Discharge 2(L3)");
    case 0x8A:
      return F("Over Discharge 2(L4)");
    case 0x8B:
      return F("Over Discharge 2(L5)");
    case 0x8C:
      return F("Over Discharge 3");
    case 0x8D:
      return F("Over Discharge 4");
    case 0x8E:
      return F("Over Discharge 5");
    case 0x8F:
      return F("Over Discharge 6");
    case 0x90:
      return F("Over Current(18V)");
    case 0x91:
      return F("Over Current(MC less) 1");
    case 0x92:
      return F("Over Current(MC less) 2");
    case 0x93:
      return F("Abnormal Current");
    case 0x94:
      return F("Over Current(High Burden) 1");
    case 0x95:
      return F("Over Current(High Burden) 2");
    case 0x97:
      return F("Overtemp upper(Discharge)");
    case 0x98:
      return F("Overtemp lower(Discharge)");
    case 0x99:
      return F("Upper Temp 60 degrees");
    case 0x9A:
      return F("Upper Temp 65 degrees");
    case 0x9B:
      return F("Upper Temp 70 degrees");
    case 0x9C:
      return F("Upper Temp 75 degrees");
    case 0x9D:
      return F("Lower Temp 60 degrees");
    case 0x9E:
      return F("Lower Temp 65 degrees");
    case 0x9F:
      return F("Lower Temp 70 degrees");
    case 0xA0:
      return F("Lower Temp 75 degrees");
    case 0xA2:
      return F("Partial Bank Charge(Upper C)");
    case 0xA3:
      return F("Partial Bank Charge(Lower C)");
    case 0xA4:
      return F("Partial Bank Charge(Upper -)");
    case 0xA5:
      return F("Partial Bank Charge(Lower -)");
    case 0xA7:
      return F("Cell_TH Disconnect(Upper Bank)");
    case 0xA8:
      return F("Cell_TH Disconnect(Lower Bank)");
    case 0xA9:
      return F("Cell_TH Short(Upper Bank)");
    case 0xAA:
      return F("Cell_TH Short(Lower Bank)");
    case 0xAC:
      return F("Current Detect Error");
    case 0xAD:
      return F("Cell Voltage Detect Error");
    case 0xAE:
      return F("VIN12 Voltage Detect Error");
    case 0xAF:
      return F("TH Resistor Detect Error");
    case 0xB0:
      return F("Upper Bank Voltage Detect Error");
    case 0xB2:
      return F("Bank Voltage Unbalance L > U");
    case 0xB3:
      return F("Bank Voltage Unbalance L < U");
    case 0xB4:
      return F("Too Bad Balance");
    case 0xB6:
      return F("Temperature Unbalance L > U");
    case 0xB7:
      return F("Temperature Unbalance L < U");
    case 0xB9:
      return F("Cell Unbalance");
    case 0xBA:
      return F("Intermittent Shorts(Upper)");
    case 0xBB:
      return F("Intermittent Shorts(Lower)");
    case 0xBC:
      return F("Intermittent Shorts(Lower)");
    case 0xBD:
      return F("Abnormally High Temperature");
    case 0xBE:
      return F("Extreme Overdischarge");
    case 0xC9:
      return F("Protect High Temperature 1");
    case 0xCA:
      return F("Protect High Temperature 2");
    case 0xCE:
      return F("Protect Over Current 1");
    case 0xCF:
      return F("Protect Over Current 2");
    case 0xD3:
      return F("Protect Low Voltage 1");
    case 0xD4:
      return F("Protect Low Voltage 2");
    case 0xD8:
      return F("Protect Over Voltage 1");
    case 0xD9:
      return F("Protect Over Voltage 2");
    case 0xDD:
      return F("Broken Detect 1");
    case 0xDE:
      return F("Broken Detect 2");
  }

  return F("Unknown");
  // just output error code
}

const __FlashStringHelper* longErrorType(uint16_t typeCode) {
  switch (typeCode) {
    case 0x01:
      return F("Upper Side Over Charge");
    case 0x02:
      return F("Lower Side Over Charge cell 1");
    case 0x03:
      return F("Lower Side Over Charge cell 2");  //     return (errorCode==0x42 || errorCode==0xE0 || errorCode==0x80 || errorCode==0x13 || errorCode==0x3);
    case 0x04:
      return F("Lower Side Over Charge cell 3");
    case 0x05:
      return F("Lower Side Over Charge cell 4");
    case 0x06:
      return F("Lower Side Over Charge cell 5");
    case 0x08:
      return F("Over Charge 2");
    case 0x09:
      return F("Over Charge 2(NOT Over Charge)");
    case 0x0B:
      return F("Upper Side Cell High Temperature(Charge)");
    case 0x0C:
      return F("Lower Side Cell High Temperature(Charge)");
    case 0x0E:
      return F("Partial Bank Charge(Upper C Bank)");
    case 0x0F:
      return F("Partial Bank Charge(Lower C Bank)");
    case 0x10:
      return F("Partial Bank Charge(Upper - Bank)");
    case 0x11:
      return F("Partial Bank Charge(Lower - Bank)");
    case 0x13:
      return F("Over Charge Current");
    case 0x15:
      return F("Abnormal Cell(Upper Side)");
    case 0x16:
      return F("Abnormal Cell(Lower Side cell 1)");
    case 0x17:
      return F("Abnormal Cell(Lower Side cell 2)");
    case 0x18:
      return F("Abnormal Cell(Lower Side cell 3)");
    case 0x19:
      return F("Abnormal Cell(Lower Side cell 4)");
    case 0x1A:
      return F("Abnormal Cell(Lower Side cell 5)");
    case 0x1C:
      return F("Over Discharge 1(Upper Side)");
    case 0x1D:
      return F("Over Discharge 1(Lower Side cell 1)");
    case 0x1E:
      return F("Over Discharge 1(Lower Side cell 2)");
    case 0x1F:
      return F("Over Discharge 1(Lower Side cell 3)");
    case 0x20:
      return F("Over Discharge 1(Lower Side cell 4)");
    case 0x21:
      return F("Over Discharge 1(Lower Side cell 5)");
    case 0x22:
      return F("Over Discharge 2(Upper Side)");
    case 0x23:
      return F("Over Discharge 2(Lower Side cell 1)");
    case 0x24:
      return F("Over Discharge 2(Lower Side cell 2)");
    case 0x25:
      return F("Over Discharge 2(Lower Side cell 3)");
    case 0x26:
      return F("Over Discharge 2(Lower Side cell 4)");
    case 0x27:
      return F("Over Discharge 2(Lower Side cell 5)");
    case 0x28:
      return F("Over Discharge 3");
    case 0x29:
      return F("Over Discharge 4");
    case 0x2A:
      return F("Over Discharge 5");
    case 0x2B:
      return F("Over Discharge 6");
    case 0x33:
      return F("Upper Side Cell High Temperature(Discharge)");
    case 0x34:
      return F("Lower Side Cell High Temperature(Discharge)");
    case 0x35:
      return F("Upper Side Cell Temperature Boosted 60 degrees");
    case 0x36:
      return F("Upper Side Cell Temperature Boosted 65 degrees");
    case 0x37:
      return F("Upper Side Cell Temperature Boosted 70 degrees");
    case 0x38:
      return F("Upper Side Cell Temperature Boosted 75 degrees");
    case 0x39:
      return F("Lower Side Cell Temperature Boosted 60 degrees");
    case 0x3A:
      return F("Lower Side Cell Temperature Boosted 65 degrees");
    case 0x3B:
      return F("Lower Side Cell Temperature Boosted 70 degrees");
    case 0x3C:
      return F("Lower Side Cell Temperature Boosted 75 degrees");
    case 0x43:
      return F("Cell_TH Disconnect(Upper Bank)");
    case 0x44:
      return F("Cell_TH Disconnect(Lower Bank)");
    case 0x45:
      return F("Cell_TH Short(Upper Bank)");
    case 0x46:
      return F("Cell_TH Short(Lower Bank)");
    case 0x48:
      return F("Current Detect Error");
    case 0x49:
      return F("Cell Voltage Detect Error");
    case 0x4A:
      return F("VIN12 Voltage Detect Error");
    case 0x4B:
      return F("TH Resistor Detect Error");
    case 0x4C:
      return F("Upper Bank Voltage Detect Error");
    case 0x4E:
      return F("Bank Voltage Unbalance L > U");
    case 0x4F:
      return F("Bank Voltage Unbalance L < U");
    case 0x50:
      return F("Too Bad Balance");
    case 0x52:
      return F("Temperature Unbalance L > U");
    case 0x53:
      return F("Temperature Unbalance L < U");
    case 0x55:
      return F("Cell Unbalance");
    case 0x59:
      return F("Abnormally High Temperature");
    case 0x5A:
      return F("Extreme Overdischarge");
    case 0x65:
      return F("Over Charge 1(Upper Side)");
    case 0x66:
      return F("Over Charge 1(Lower Side cell 1)");
    case 0x67:
      return F("Over Charge 1(Lower Side cell 2)");
    case 0x68:
      return F("Over Charge 1(Lower Side cell 3)");
    case 0x69:
      return F("Over Charge 1(Lower Side cell 4)");
    case 0x6A:
      return F("Over Charge 1(Lower Side cell 5)");
    case 0x6C:
      return F("Over Charge 2");
    case 0x6D:
      return F("Over Charge 2(NOT Over Charge 1)");
    case 0x80:
      return F("Over Discharge 1(Upper Side)");
    case 0x81:
      return F("Over Discharge 1(Lower Side cell 1)");
    case 0x82:
      return F("Over Discharge 1(Lower Side cell 2)");
    case 0x83:
      return F("Over Discharge 1(Lower Side cell 3)");
    case 0x84:
      return F("Over Discharge 1(Lower Side cell 4)");
    case 0x85:
      return F("Over Discharge 1(Lower Side cell 5)");
    case 0x86:
      return F("Over Discharge 2(Upper Side)");
    case 0x87:
      return F("Over Discharge 2(Lower Side cell 1)");
    case 0x88:
      return F("Over Discharge 2(Lower Side cell 2)");
    case 0x89:
      return F("Over Discharge 2(Lower Side cell 3)");
    case 0x8A:
      return F("Over Discharge 2(Lower Side cell 4)");
    case 0x8B:
      return F("Over Discharge 2(Lower Side cell 5)");
    case 0x8C:
      return F("Over Discharge 3");
    case 0x8D:
      return F("Over Discharge 4");
    case 0x8E:
      return F("Over Discharge 5");
    case 0x8F:
      return F("Over Discharge 6");
    case 0x90:
      return F("Over Current(18V)");
    case 0x91:
      return F("Over Current(MC less) 1");
    case 0x92:
      return F("Over Current(MC less) 2");
    case 0x93:
      return F("Abnormal Current");
    case 0x94:
      return F("Over Current(High Burden) 1");
    case 0x95:
      return F("Over Current(High Burden) 2");
    case 0x97:
      return F("Upper Side Cell High Temperature(Discharge)");
    case 0x98:
      return F("Lower Side Cell High Temperature(Discharge)");
    case 0x99:
      return F("Upper Side Cell Temperature Boosted 60 degrees");
    case 0x9A:
      return F("Upper Side Cell Temperature Boosted 65 degrees");
    case 0x9B:
      return F("Upper Side Cell Temperature Boosted 70 degrees");
    case 0x9C:
      return F("Upper Side Cell Temperature Boosted 75 degrees");
    case 0x9D:
      return F("Lower Side Cell Temperature Boosted 60 degrees");
    case 0x9E:
      return F("Lower Side Cell Temperature Boosted 65 degrees");
    case 0x9F:
      return F("Lower Side Cell Temperature Boosted 70 degrees");
    case 0xA0:
      return F("Lower Side Cell Temperature Boosted 75 degrees");
    case 0xA2:
      return F("Partial Bank Charge(Upper C Bank)");
    case 0xA3:
      return F("Partial Bank Charge(Lower C Bank)");
    case 0xA4:
      return F("Partial Bank Charge(Upper - Bank)");
    case 0xA5:
      return F("Partial Bank Charge(Lower - Bank)");
    case 0xA7:
      return F("Cell_TH Disconnect(Upper Bank)");
    case 0xA8:
      return F("Cell_TH Disconnect(Lower Bank)");
    case 0xA9:
      return F("Cell_TH Short(Upper Bank)");
    case 0xAA:
      return F("Cell_TH Short(Lower Bank)");
    case 0xAC:
      return F("Current Detect Error");
    case 0xAD:
      return F("Cell Voltage Detect Error");
    case 0xAE:
      return F("VIN12 Voltage Detect Error");
    case 0xAF:
      return F("TH Resistor Detect Error");
    case 0xB0:
      return F("Upper Bank Voltage Detect Error");
    case 0xB2:
      return F("Bank Voltage Unbalance L > U");
    case 0xB3:
      return F("Bank Voltage Unbalance L < U");
    case 0xB4:
      return F("Too Bad Balance");
    case 0xB6:
      return F("Temperature Unbalance L > U");
    case 0xB7:
      return F("Temperature Unbalance L < U");
    case 0xB9:
      return F("Cell Unbalance");
    case 0xBA:
      return F("Intermittent Shorts(Upper Bank)");
    case 0xBB:
      return F("Intermittent Shorts(Lower Bank)");
    case 0xBC:
      return F("Intermittent Shorts(Lower Bank)");
    case 0xBD:
      return F("Abnormally High Temperature");
    case 0xBE:
      return F("Extreme Overdischarge");
    case 0xC9:
      return F("Protect High Temperature 1");
    case 0xCA:
      return F("Protect High Temperature 2");
    case 0xCE:
      return F("Protect Over Current 1");
    case 0xCF:
      return F("Protect Over Current 2");
    case 0xD3:
      return F("Protect Low Voltage 1");
    case 0xD4:
      return F("Protect Low Voltage 2");
    case 0xD8:
      return F("Protect Over Voltage 1");
    case 0xD9:
      return F("Protect Over Voltage 2");
    case 0xDD:
      return F("Broken Detect 1");
    case 0xDE:
      return F("Broken Detect 2");
  }

  return F("Unknown");
  // just output error code
}

const __FlashStringHelper* toolType(uint16_t type) {
  switch (type) {
    case 0x0001:
      return F("H60MEY");
    case 0x0002:
      return F("W4/5SE");
    case 0x0003:
      return F("G12VE");
    case 0x0004:
      return F("DH40MEY");
    case 0x0005:
      return F("DH45MEY");
    case 0x0006:
      return F("DH52MEY");
    case 0x0007:
      return F("H45MEY");
    case 0x0008:
      return F("G10VE/G13VE");
    case 0x0009:
      return F("G10YE2/G13YE2");
    case 0x000A:
      return F("G13VE");
    case 0x000B:
      return F("G10BVE");
    case 0x000C:
      return F("G10BVE");
    case 0x000D:
      return F("G18DBBVL/G14DBBVL");
    case 0x000E:
      return F("G18DBBL/G18DBBAL");
    case 0x000F:
      return F("WR25SE");
    case 0x0010:
      return F("WR25SE");
    case 0x0011:
      return F("WR22SE");
    case 0x0012:
      return F("WR22SE");
    case 0x0013:
      return F("WR25SE");
    case 0x0014:
      return F("WR25SE");
    case 0x0015:
      return F("WR25SE");
    case 0x0016:
      return F("WR25SE");
    case 0x0017:
      return F("WR22SE");
    case 0x0018:
      return F("WR22SE");
    case 0x0019:
      return F("WR22SE");
    case 0x001D:
      return F("WR22SE");
    case 0x001E:
      return F("CR14/18DBL");
    case 0x001F:
      return F("C6MEY(S)");
    case 0x0020:
      return F("G18BYE");
    case 0x0021:
      return F("G18BYE");
    case 0x0022:
      return F("G18BYE");
    case 0x0023:
      return F("H41SE/ME");
    case 0x0024:
      return F("DH28PEC");
    case 0x2711:
      return F("36V Tool");
    case 0x2712:
      return F("BL18V Tool");
    case 0x2713:
      return F("18V Tool");
    case 0x2714:
      return F("G3610DA");
    case 0x2715:
      return F("C3606DA");
    case 0x2716:
      return F("C3606DRA");
    case 0x2717:
      return F("WH36DA");
    case 0x2718:
      return F("DS/DV36DA");
    case 0x2719:
      return F("DH36DPA");
    case 0x271A:
      return F("DH36DMA/DSA");
    case 0x271B:
      return F("C18DBAL_UL");
    case 0x271C:
      return F("WH36DB");
    case 0x271D:
      return F("WR36DA/B");
    case 0x271E:
      return F("CR36DA");
    case 0x271F:
      return F("C3610DRA");
    case 0x2720:
      return F("C3607DA");
    case 0x2721:
      return F("RP3608DB");
    case 0x2722:
      return F("C3607DRA");
    case 0x2723:
      return F("NT3640DA");
    case 0x2724:
      return F("RP3608DA");
    case 0x2725:
      return F("DS12DD");
    case 0x2726:
      return F("DB12DD");
    case 0x2727:
      return F("DV12DD");
    case 0x2728:
      return F("WH12DD");
    case 0x2729:
      return F("CD12DA");
    case 0x272A:
      return F("CJ12DA");
    case 0x272B:
      return F("CR12DA");
    case 0x272C:
      return F("SV12DA");
    case 0x272D:
      return F("DH12DA");
    case 0x272E:
      return F("C3605DA");
    case 0x272F:
      return F("C3605DA(BT)");
    case 0x2730:
      return F("CD3605DA");
    case 0x2731:
      return F("C3605DC");
    case 0x2732:
      return F("C3605DC(BT)");
    case 0x2733:
      return F("N3604/10DJ");
    case 0x2734:
      return F("CJ36DA");
    case 0x2735:
      return F("CJ36DB");
    case 0x2736:
      return F("VB3616DA");
    case 0x2737:
      return F("G3618/23DA");
    case 0x2738:
      return F("CD3607DA");
    case 0x2739:
      return F("UV3628DA");
    case 0x273A:
      return F("NR3675DD/NR3665DA");
    case 0x273B:
      return F("W36DYA");
    case 0x273C:
      return F("CB3612DA");
    case 0x273D:
      return F("WH36DC");
    case 0x273E:
      return F("UM36DA");
    case 0x273F:
      return F("R36DA");
    case 0x2740:
      return F("D3613DA");
    case 0x2741:
      return F("EC36DA");
    case 0x2742:
      return F("CSM-R(TONE)");
    case 0x2743:
      return F("DH36DPE");
    case 0x2744:
      return F("WR36DD");
    case 0x2745:
      return F("CS3630DB/CS3635DB");
    case 0x2746:
      return F("CG36DB");
    case 0x2747:
      return F("M1808DA");
    case 0x2748:
      return F("M3608DA");
    case 0x2749:
      return F("M3612DA");
    case 0x274A:
      return F("NT1865DA");
    case 0x274B:
      return F("CKS-W(TONE)");
    case 0x274C:
      return F("N1804DA/NT1850DF");
    case 0x274D:
      return F("GP36DA/DB");
    case 0x274E:
      return F("RB36DB");
    case 0x274F:
      return F("C3606DPA");
    case 0x2750:
      return F("CD3605DB");
    case 0x2751:
      return F("WR36DE");
    case 0x2752:
      return F("C1807DA");
    case 0x2753:
      return F("C3607DWA");
    case 0x2754:
      return F("WH36DC(Functional safety)");
    case 0x2755:
      return F("N3604DM");
    case 0x2756:
      return F("WH12DCA");
    case 0x2757:
      return F("WH18DC");
    case 0x2758:
      return F("DH3628DA");
    case 0x2759:
      return F("DS36DC(Functional safety)");
    case 0x275A:
      return F("DC18DC");
    case 0x275B:
      return F("CS3625DC/CS3630DC");
    case 0x275C:
      return F("CR36DYA");
    case 0x275D:
      return F("CR18DMA");
    case 0x275E:
      return F("CR36DMA");
    case 0x275F:
      return F("DH3640DA");
    case 0x2760:
      return F("C3606DB");
    case 0x2761:
      return F("DS36DC");
    case 0x2762:
      return F("WR36DF");
    case 0x2763:
      return F("ML36DB");
    case 0x2764:
      return F("WR36DG");
    case 0x2765:
      return F("SB3608DA");
    case 0x2766:
      return F("C1806DC");
    case 0x2767:
      return F("WR36DC");
    case 0x2768:
      return F("G3610DC");
    case 0x2769:
      return F("G1810DB");
    case 0x276A:
      return F("G1810DA");
    case 0x276B:
      return F("WR36DH");
    case 0x276C:
      return F("WR36DH");
    case 0x276D:
      return F("G1210DA");
    case 0x276E:
      return F("CG36DC");
    case 0x276F:
      return F("N1804DA");
    case 0x2770:
      return F("D36DYA");
    case 0x2771:
      return F("NP3635DA");
    case 0x2772:
      return F("UL18DD");
    case 0x2773:
      return F("UL18DE");
    case 0x2774:
      return F("C1805DB");
    case 0x2775:
      return F("C3607DB");
    case 0x2776:
      return F("C3612DRA");
    case 0x2777:
      return F("WH18DP");
    case 0x2778:
      return F("DS/DV18DP");
    case 0x2779:
      return F("H3641DA/DB");
    case 0x277A:
      return F("NT1850DG/N1804DB");
    case 0x277B:
      return F("EC4516HY");
    case 0x277C:
      return F("C1805DA/DB");
    case 0x277D:
      return F("WHP18DA");
    case 0x277E:
      return F("WHP12DA");
    case 0x277F:
      return F("WH36DD");
    case 0x2780:
      return F("WR36DF");
    case 0x2781:
      return F("CE12DA");
    case 0x2782:
      return F("CN12DA");
    case 0x2783:
      return F("X389");
    case 0x2784:
      return F("SAMPLE FOR CACTUS");
    case 0x2785:
      return F("SAMPLE FOR CACTUS");
    case 0x2786:
      return F("C1205DA");
    case 0x2787:
      return F("CD1205DA");
    case 0x2788:
      return F("W12DA");
    case 0x2789:
      return F("N1812DA/N1810DA");
    case 0x278A:
      return F("N1850DAA/DSA");
    case 0x278B:
      return F("SDVR-01");
    case 0x278C:
      return F("NR1890DCA");
    case 0x278D:
      return F("NR1865DAA");
    case 0x278E:
      return F("NT1865DSA");
    case 0x278F:
      return F("NV1845DA");
    case 0x2790:
      return F("C3609DUM");
    case 0x2791:
      return F("NR1838DKA");
    case 0x2792:
      return F("CR36DSA");
    case 0x2793:
      return F("CV36DMA");
  }
}

int doCmd(char* cmd, uint16_t returnValues[], bool isShort=false) {
  uint8_t buffer[512];
  uint8_t* rpy[64];
  int readLen = isShort?32:512;

  for (int retry = 0; retry < 5; retry++) {
    memset(returnValues, 0, 64 * 2);
    memset(buffer, 0, readLen);
    serial1.write((uint8_t*)cmd, strlen(cmd));
    delay(1);
    serial1.read(buffer, readLen);

    int split_cnt = split_to('\r', buffer, 512, rpy, 64);

    if (split_cnt > 1) {
			Serial.println(cmd);
      for (int i = 1; i < split_cnt; i++) {
        returnValues[i - 1] = atoi((char*)rpy[i]);
				Serial.println(String((i-1)) + String(" : ") + String((char*)rpy[i]) + "  -  " + String(returnValues[i-1]));
      }
      return max(split_cnt - 1, 0);
    }
  }
  return 0;
}

double getCapacity(double voltage) {
  double voltages[] = {14.5f, 16.0f, 16.935f, 17.93f, 18.705f, 19.41f, 20.25f};
  double percentages[] = {0.0f, 10.0f, 20.0f, 40.0f, 60.0f, 80.0f, 100.0f};

  if (voltage <= voltages[0]) {
    return 0.0;
  }

  if (voltage >= voltages[6]) {
    return 100.0;
  }

  double capacity = 0.0;
  for (int i = 0; i < 5; i++) {
    if (voltage <= voltages[i + 1]) {
      double gradient = (voltages[i + 1] - voltages[i]) / (percentages[i + 1] - percentages[i]);
      double intercept = voltages[i] - gradient * percentages[i];
      capacity = (voltage - intercept) / gradient;
      break;
    }
  }
  return capacity;
}


void showDone() {
  DinMeter.Display.clear();
  DinMeter.Display.drawString("done.", 20, 20);
  delay(2000);
}


void showWorking() {
  DinMeter.Display.clear();
  DinMeter.Display.drawString("Working....", 20, 20);
}


void showCommunicationError() {
  DinMeter.Display.clear();
  DinMeter.Display.drawString("Communication Error.", 20, 20);
}


void getFirstScreenInfo() {
  uint16_t r1001[64];
  uint16_t r1004[64];
  uint16_t r1010[64];
	showWorking();

  doCmd("\r\n", r1001,true);  // first command is ignored

  if(!(doCmd("R1010\r\n", r1010)>0 && doCmd("R1001\r\n", r1001) >0 && doCmd("R1004\r\n", r1004)>0)){
		showCommunicationError();
		return;
	}

	DinMeter.Display.clear();


  char batteryModel[32] = {0};
  char batterySerial[32] = {0};
  char manufactureDate[32] = {0};
  char wakeTime[32] = {0};
  char work36VTime[32] = {0};
  char work18VTime[32] = {0};

  strcpy_P(batteryModel, (const char*)batteryType(r1001[3]));
  sprintf(batterySerial, "%02u%02u%02u%02u%02u%02u", r1001[9], r1001[10], r1001[11], r1001[12], r1001[13], r1001[14]);
  sprintf(manufactureDate, "%02u-%02u-%04u ", r1001[11], r1001[10], r1001[9] + 2000);
  sprintf(wakeTime, "%04u:%02u", r1010[4], r1010[5]);
  sprintf(work36VTime, "%04u:%02u", r1010[7], r1010[8]);
  sprintf(work18VTime, "%04u:%02u", r1010[10] + r1010[13], r1010[11] + r1010[14]);

  DinMeter.Display.drawString("Model: " + String(batteryModel), 5, 3);
  DinMeter.Display.drawString("Serial: " + String(batterySerial), 5, 18);
  DinMeter.Display.drawString("Charge count: " + String(r1010[3]), 5, 33);
  DinMeter.Display.drawString("Mfg date: " + String(manufactureDate), 5, 48);
  DinMeter.Display.drawString("Wake time: " + String(wakeTime) + "h", 5, 63);
  DinMeter.Display.drawString("18v: " + String(work18VTime) + "h", 5, 78);
  DinMeter.Display.drawString("36v: " + String(work36VTime) + "h", 5, 93);
  DinMeter.Display.drawString("Charge: " + String(getCapacity((r1004[5] + r1004[11]) / 2000.0f)) + "%", 5, 108);
}

void getSecondScreenInfo() {
  uint16_t r1004[64];
  uint16_t r1009[64];
	showWorking();

  doCmd("\r\n", r1004,true);  // first command is ignored

	if(!(doCmd("R1004\r\n", r1004)>0 && doCmd("R1009\r\n", r1009)>0)){
		showCommunicationError();
		return;
	}

	DinMeter.Display.clear();
  DinMeter.Display.drawString("LowerBank: " + String(r1004[5] / 1000.0f) + "v", 5, 3);
  DinMeter.Display.drawString("UpperBank: " + String(r1004[11] / 1000.0f) + "v", 5, 18);
  DinMeter.Display.drawString("UnbalanceLower: " + String(r1009[27] / 1000.0f), 5, 33);
  DinMeter.Display.drawString("UnbalanceUpper: " + String(r1009[28] / 1000.0f), 5, 48);
  DinMeter.Display.drawString("TLower: " + String(pow(-1.0f, r1004[12]) * r1004[13]) + "c", 5, 63);
  DinMeter.Display.drawString("TUpper: " + String(pow(-1.0f, r1004[14]) * r1004[15]) + "c", 5, 78);
  DinMeter.Display.drawString("L1: " + String(r1004[6] / 1000.0f) + " L2: " + String(r1004[7] / 1000.0f), 5, 93);
  DinMeter.Display.drawString("L3: " + String(r1004[8] / 1000.0f) + " L4: " + String(r1004[9] / 1000.0f), 5, 108);
  DinMeter.Display.drawString("L5: " + String(r1004[10] / 1000.0f), 5, 123);
}

void getThirdScreenInfo() {
  uint16_t r1003[64];
  char errorLine[256] = {0};
  int count = 1;
	showWorking();

  doCmd("\r\n", r1003,true);  // first command is ignored

	if(!(doCmd("R1003\r\n", r1003)>0 )){
		showCommunicationError();
		return;
	}

  DinMeter.Display.clear();
  DinMeter.Display.setTextFont(&fonts::Font0);
  DinMeter.Display.drawString("Fatal | Charge No | Message", 5, 3);

  for (int i = 0; i < 10; i++) {
    int code = r1003[i + 9];
    if (code) {
      sprintf_P(errorLine, (const char*)errorType(code));
      char* fatal = "N";
      if ((code == 8 || code == 9 || code == 108 || code == 109)) {
        fatal = "Y";
      }
      DinMeter.Display.drawString(String(fatal) + " | " + String(r1003[29 + i] + (i == 0 ? 0 : r1003[29])) + " | " + String(errorLine), 5, 3 + (count * 12));
      count++;
    }
  }

  DinMeter.Display.setTextFont(&fonts::FreeMono9pt7b);
}

void getFourthScreenInfo() {
  uint16_t r1010[64];

	showWorking();
  doCmd("\r\n", r1010,true);  // first command is ignored

	if(!(doCmd("R1010\r\n", r1010)>0 )){
		showCommunicationError();
		return;
	}

  char toolLine[64] = {0};
  int count = 1;

  DinMeter.Display.clear();
  DinMeter.Display.setTextFont(&fonts::Font0);
  DinMeter.Display.drawString("Usage per tool", 5, 3);

  for (int i = 0; i < 10; i++) {
    int code = r1010[i + 16];
    if (code) {
      sprintf_P(toolLine, (const char*)toolType(code));
      DinMeter.Display.drawString(String(toolLine) + ": " + String(r1010[i + 26]), 5, 3 + (count * 15));
      count++;
    }
  }
  DinMeter.Display.setTextFont(&fonts::FreeMono9pt7b);
}

void getFifthScreenInfo() {
  uint16_t r1001[64];
  uint16_t r1003[64];
  uint16_t r1004[64];
  uint16_t r1009[64];

	showWorking();
  
	doCmd("\r\n", r1001,true);  // first command is ignored
	if(!(doCmd("R1001\r\n", r1001)>0 && doCmd("R1003\r\n", r1003)>0 && doCmd("R1004\r\n", r1004)>0  &&   doCmd("R1009\r\n", r1009)>0)){
		showCommunicationError();
		return;
	}

  bool brokenFlag = isBroken(r1001[3], &r1004[17]);
  bool fuseError = (r1004[25] > 0);
  bool fatalErrorsInLog = false;

  for (int i = 0; i < 10; i++) {
    int code = r1003[i + 9];
    fatalErrorsInLog |= (code == 8 || code == 9 || code == 108 || code == 109);
  }

  DinMeter.Display.clear();
  DinMeter.Display.drawString("Error flag set: " + (brokenFlag ? String("Yes") : String("No")), 5, 3);
  DinMeter.Display.drawString("Fuse Error: " + (fuseError ? String("Yes") : String("No")), 5, 18);
  DinMeter.Display.drawString("Previous overchg: " + (fatalErrorsInLog ? String("Yes") : String("No")), 5, 33);

  DinMeter.Display.drawString("Unbalanced chg: " + String(r1009[5]), 5, 48);
  DinMeter.Display.drawString("Overtemp chg: " + String(r1009[4]), 5, 63);
  DinMeter.Display.drawString("Overcurrent chg: " + String(r1009[6]), 5, 78);
}

void getSixthScreenInfo() {
  uint16_t r1009[64];

	showWorking();
  doCmd("\r\n", r1009,true);  // first command is ignored

	if(!( doCmd("R1009\r\n", r1009)>0)){
		showCommunicationError();
		return;
	}

  int totalOverdischarge = r1009[7] + r1009[8] + r1009[9] + r1009[10];

	DinMeter.Display.clear();
  DinMeter.Display.drawString(String("Overdischarge: ") + String(totalOverdischarge), 5, 3);
  DinMeter.Display.drawString(String("18v Brushless OC: ") + String(r1009[12]), 5, 18);
  DinMeter.Display.drawString(String("18v Brushed OC: ") + String(r1009[13]), 5, 33);
  DinMeter.Display.drawString(String("36v OC: ") + String(r1009[14]), 5, 48);
  DinMeter.Display.drawString(String("OC High Load: ") + String(r1009[15]), 5, 63);
  DinMeter.Display.drawString(String("Disch overtemp: ") + String(r1009[16]), 5, 78);
}


void setup() {
  auto cfg = M5.config();
  DinMeter.begin(cfg, true);
  DinMeter.Display.setRotation(1);
  DinMeter.Display.setTextColor(GREEN);
  DinMeter.Display.setTextDatum(top_left);
  DinMeter.Display.setTextFont(&fonts::FreeMono9pt7b);

  DinMeter.Display.setTextSize(1);
  DinMeter.update();

  Serial.begin(9600);
  digitalWrite(10, HIGH);
  digitalWrite(9, LOW);
}

void loop() {
  // variables we update each loop
  static int screen = 0;
  static long old_position = DinMeter.Encoder.read();
  static int autoTurnoff = -1;
  static bool nextNewScr = false;
  static int loopCount = 0;

  if (autoTurnoff < 0) {
    autoTurnoff = millis() + (TURNOFF_SECONDS * 1000);
  }

  DinMeter.update();

  long newPosition = DinMeter.Encoder.read();
  bool btnPressed = DinMeter.BtnA.wasPressed();
  bool newScr = false;
	bool firstScr=true;

  loopCount++;

  serial1.write((uint8_t*)"\x80\x80\x80\x80\x80\x80\x80\x80", 8);
  if (DinMeter.BtnA.pressedFor(5000) || (millis() > autoTurnoff)) {
    DinMeter.Power.powerOff();
    return;
  }

  if (abs(newPosition - old_position) > 2) {
    if (newPosition < old_position) {
      screen++;
    }

    if (newPosition > old_position) {
      screen--;
    }

    screen = screen < 0 ? 6 : screen;
    screen = screen > 6 ? 0 : screen;

    newScr = true;

    old_position = newPosition;
  }

  if (nextNewScr ||( firstScr &&  loopCount == 50)) {
    newScr = true;
    nextNewScr = false;
		firstScr=false;
  }

  if (newScr || btnPressed) {
    autoTurnoff = millis() + (TURNOFF_SECONDS * 1000);
  }

  switch (screen) {
    case 0:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getFirstScreenInfo();
      }
      break;

    case 1:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getSecondScreenInfo();
        // readSensors();
      }
      break;

    case 2:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getThirdScreenInfo();
      }
      break;

    case 3:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getFourthScreenInfo();
      }
      break;

    case 4:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getFifthScreenInfo();
      }
      break;

    case 5:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getSixthScreenInfo();
      }
      break;
    case 6:
      if (newScr) {
        DinMeter.Display.clear();
        DinMeter.Display.drawString("Reset lockout.", 5, 5);
      }

      if (btnPressed) {
				showWorking();

        for (int i = 0; i < 3; i++) {
            delay(300);
					  uint16_t discard[64];
						doCmd("R1012\r\n002\r\n050\r\n00050",discard,true);
						doCmd("R1012\r\n002\r\n051\r\n00051",discard,true);
        }

        showDone();
        nextNewScr = true;
      }
      break;
  }

  delay(20);
}