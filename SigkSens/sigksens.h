#ifndef _sigksens_H_
#define _sigksens_H_

#include <ReactESP.h>


extern ReactESP app;

//helper function
void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
String uuidv4();

#endif
