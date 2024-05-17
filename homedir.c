#ifdef __cplusplus
extern "C"
{
#endif

#include "homedir.h"

#define MAX_PATH 128

char *get_homedir(void){
    char homedir[MAX_PATH] = {0};

#ifdef _WIN32
    snprintf(homedir, MAX_PATH, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
    snprintf(homedir, MAX_PATH, "%s", getenv("HOME"));
#endif
    char *answer = calloc(strlen(homedir), sizeof(char) + 1);
    strcpy(answer, homedir);
    return answer;
}

#ifdef __cplusplus
}
#endif

