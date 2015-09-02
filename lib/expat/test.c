#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <expat.h>

void startElement(void *userData, const char *name, const char **atts)
{
	int i;
	printf("start(%s)\n", name, userData);
	for(i = 0; atts[i]; i+=2) {
		char *key = atts[i+0];
		char *val = atts[i+1];
		printf(" * %s = %s\n", key, val);
	}
}

void endElement(void *userData, const char *name)
{
	printf("end(%s)\n", name, userData);
}

void *charElement(void *userData, const XML_Char *s, int len)
{
	char buf[len+1];
	strcpy(buf, s);
	buf[len]='\0';
	printf(" str=(%s)\n", buf);
}

int main(int argc, char **argv)
{
  char buf[8049];
  int len;   /* len is the number of bytes in the current bufferful of data */
  int done;
  int finished = 0;
  int depth = 0;  /* nothing magic about this; the sample program tracks depth to know how far to indent. */
                  /* depth is thus going to be the user data for this parser. */
  if (argc!=2) {
	printf("Gimi the xml in argv[1] plz..\n");
	return 1;
  }

printf("STRL: '%d'\n", argc);//argv[1]);
fflush(stdout);
strcpy(buf, argv[1]);
printf("STRL: '%s'\n", buf);

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser,charElement);
 done = 0;
len = strlen(buf);
   finished = 0;

printf("%x\n", parser);

  do {
	done = 0;
    if (!XML_Parse(parser, buf, len, done)) {
	done = 1;
      //handle parse error
    }
  } while (!done);

  XML_ParserFree(parser);

  return 0;
}
