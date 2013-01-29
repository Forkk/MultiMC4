
#ifndef __XDGUSERDIRLOOKUP_H__
#define __XDGUSERDIRLOOKUP_H__

char *
xdg_user_dir_lookup_with_fallback (const char *type, const char *fallback);

char *
xdg_user_dir_lookup (const char *type);

#endif
