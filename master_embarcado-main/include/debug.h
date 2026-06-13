#ifndef H_DEBUG
#define H_DEBUG
#ifdef configENABLE_DEBUG

#define DBG_PRINTLN(fmt)               Serial.println(fmt)
#define DBG_PRINT(fmt)                 Serial.print(fmt)
#define DBG_tag(tag, fmt)          Serial.print(tag);     Serial.println(fmt)


#else

#define DBG_PRINTLN(fmt)               NOP()
#define DBG_PRINT(fmt)                 NOP()
#define DBG_tag(tag, fmt)              NOP()


#endif

#endif