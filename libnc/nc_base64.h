#if !defined(__NC_BASE64_H__)
#define __NC_BASE64_H__

#include "nc_types.h"
#include "nc_string.h"

// @defines____________________________________________________________________


// @types______________________________________________________________________


// @runtime____________________________________________________________________

// @functions__________________________________________________________________
Str8 B64Decode(Str8 String);
void B64Decode(Str8 String, OUT u8* Decoded, u64 Padding);

#endif // __NC_BASE64_H__
