#include "serconfig.h"



void serconfig::settingsCommand(serbuf& sb, db1k& db, boolean attached) {
//void settingsCommand(serbuf sb, db1k db) {
      char names[3];  names[0] = '-'; names[1] = '-'; names[2] = '-';
      boolean done = false;
      for (char ix = 0; ix < 3 && !done ; ix++ ) {
        char b1 = sb.getCh();
        if (b1 >= '-') {
          names[ix] = b1;
        }
        else {
          sb.ungetCh();
          done = true;
        }
      }
      Base40 expname (names[0], names[1], names[2]);
      expname.decode();
      char exptype = sb.getCh();     // get item type
      bool dbdelete = false;  // not deleting anything
      long expvalue = 0;          // get item value
      for (char v = sb.getCh(); v != 0; v = sb.getCh()) {
        switch (exptype) {
          case '$' :
                //IFECHO Serial << exptype << v;
                if (isHexadecimalDigit(v)) {
                  expvalue <<= 4;
                  if (isLowerCase(v)) {
                    expvalue += (v - 87); // 86 = ` - 9
                  }  // abcdef
                  else if (isUpperCase(v)) {
                    expvalue += (v - 55); // 55 = @ - 9
                  }  // ABCDEF
                  else if (isDigit(v)) {
                    expvalue += (v - 48);
                  }  // 0123456789
                  //Serial << expvalue << v;
                }
              break;
          case '#' :
                //IFECHO Serial << exptype << v;
                if (isDigit(v)) {expvalue = expvalue * 10 + v - '0';}  // 0123456789
              break;
          case '!' :
                //IFECHO Serial << exptype << v;
                if ('x' == v) {dbdelete = true; exptype = '\'';}  // !key!x and done
              break;
          case '"' :
                //IFECHO Serial << exptype << v << " ";
                if (v == '"') {exptype = '\'';}                // endquote
                else {
                  if (v >= ' ') {
                    expvalue <<= 8;
                    expvalue += v;
                  }  // alphanumeric - 4 char max
                }
                //IFECHO Serial << expvalue;
              break;
          case '\'' : // done with text
              break;
          default :
                if (attached) Serial << F(" oops[") << exptype << F("]");
              break;
        }
      }
      // dump results
      if (dbdelete) {
        bool result = db.deleteKeyValue(expname._base40);
      }
      else {
        bool result = db.writeKeyValue(expname._base40, expvalue);
      }
}



