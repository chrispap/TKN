#ifndef TKNUTIL_H
#define TKNUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes */
int getKey (char *);

#ifdef __linux__
int getch();
#endif

#endif

#ifdef __cplusplus
}
#endif
