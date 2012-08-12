#ifndef _BSPATCH_H
#define _BSPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * patch oldfile by using patchfile and write the output to newfile.
 *
 * Returns 1 in case of any error. 0 otherwise.
 */
int bspatch(const char * oldfile, const char * newfile, const char * patchfile);

#ifdef __cplusplus
}
#endif


#endif
