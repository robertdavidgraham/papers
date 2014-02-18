/*
    Example of a state-machine parser
*/
#include <stdio.h>
#include <string.h>

static const char WHITESPACE[] = " \r\n\t"; 

int http_parse(int state, unsigned char c)
{
    enum States {START, METHOD, SPACE1, URI,
        SPACE2, VERSION, EOL, NAME, COLON, VALUE,
        END};
    switch (state) {
        case START:
            if (strchr(WHITESPACE, c))
                break;
            state = METHOD;
            /* drop down */
        case METHOD: /*GET, POST, HEAD, ...*/
            if (c == '\n') {
                return EOL;
            } else if (strchr(WHITESPACE, c)) {
                return SPACE1;
            } else {
                return METHOD; /* no transition */
            }
            break;
        case SPACE1:
            if (c == '\n') {
                return EOL;
            } else if (strchr(WHITESPACE, c)) {
                return SPACE1;
            }
            state = URI;
            /* drop down */
        case URI: 
            if (c == '\n') {
                return EOL;
            } else if (strchr(WHITESPACE, c)) {
                return SPACE2;
            } else {
                return URI; /* no transition */
            }
            break;
        case SPACE2:
            if (c == '\n') {
                return EOL;
            } else if (strchr(WHITESPACE, c)) {
                return SPACE1;
            }
            state = VERSION;
            /* drop down */
        case VERSION:
            if (c == '\n') {
                return EOL;
            } else if (strchr(WHITESPACE, c)) {
                return VERSION; /* error */
            } else {
                return VERSION; /* no transition */
            }
            break;
        case EOL:
            if (c == '\r')
                return EOL;
            else if (c == '\n')
                return END;
            state = NAME;
            /* drop down */
        case NAME:
            if (c == '\n') {
                return EOL;
            } else if (c == ':') {
                return COLON;
            } else {
                return NAME; /* no transition */
            }
            break;
        case COLON:
            if (c == '\n') {
                return EOL;
            } else if (strchr(WHITESPACE, c)) {
                return COLON;
            }
            state = VALUE;
            /* drop down */
        case VALUE:
            if (c == '\n') {
                return EOL;
            } else {
                return VALUE; /* no transition */
            }
            break;
        case END:
            return END;
    }
}

int main(int argc, char *argv[])
{
    int state = 0;
    
    while (!feof(stdin)) {
        int c = getc(stdin);
        state = http_parse(state, (unsigned char)c);
        printf("%c", c);
    }
    return 0;
}

