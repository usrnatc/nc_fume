#if !defined(__NC_HTTP_H__)
#define __NC_HTTP_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_system.h"
#include "nc_ring.h"

// @defines____________________________________________________________________
enum HTTPStatusKind {
    HTTP_STATUS_KIND_NULL,
    HTTP_STATUS_KIND_INFO,
    HTTP_STATUS_KIND_REDIRECT,
    HTTP_STATUS_KIND_CLIENT_ERROR,
    HTTP_STATUS_KIND_SERVER_ERROR,
    HTTP_STATUS_KIND_SUCCESS,
    HTTP_STATUS_KIND_COUNT
};

typedef u32 HTTPStatusCode;
enum : u32 {
    HTTP_STATUS_CODE_CONTINUE                        = 100,
    HTTP_STATUS_CODE_SWITCHING_PROTOCOLS             = 101,
    HTTP_STATUS_CODE_PROCESSING                      = 102,
    HTTP_STATUS_CODE_EARLY_HINTS                     = 103,
    HTTP_STATUS_CODE_FIRST_INFORMATIONAL             = 100,
    HTTP_STATUS_CODE_LAST_INFORMATIONAL              = 199,

    HTTP_STATUS_CODE_OK                              = 200,
    HTTP_STATUS_CODE_CREATED                         = 201,
    HTTP_STATUS_CODE_ACCEPTED                        = 202,
    HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION   = 203,
    HTTP_STATUS_CODE_NO_CONTENT                      = 204,
    HTTP_STATUS_CODE_RESET_CONTENT                   = 205,
    HTTP_STATUS_CODE_PARTIAL_CONTENT                 = 206,
    HTTP_STATUS_CODE_MULTI_STATUS                    = 207,
    HTTP_STATUS_CODE_ALREADY_REPORTED                = 208,
    HTTP_STATUS_CODE_IM_USED                         = 226,
    HTTP_STATUS_CODE_FIRST_SUCCESSFUL                = 200,
    HTTP_STATUS_CODE_LAST_SUCCESSFUL                 = 299,

    HTTP_STATUS_CODE_MULTIPLE_CHOICES                = 300,
    HTTP_STATUS_CODE_MOVED_PERMANENTLY               = 301,
    HTTP_STATUS_CODE_FOUND                           = 302,
    HTTP_STATUS_CODE_SEE_OTHER                       = 303,
    HTTP_STATUS_CODE_NOT_MODIFIED                    = 304,
    HTTP_STATUS_CODE_USE_PROXY                       = 305,
    HTTP_STATUS_CODE_TEMPORARY_REDIRECT              = 307,
    HTTP_STATUS_CODE_PERMANENT_REDIRECT              = 308,
    HTTP_STATUS_CODE_FIRST_REDIRECTION               = 300,
    HTTP_STATUS_CODE_LAST_REDIRECTION                = 399,

    HTTP_STATUS_CODE_BAD_REQUEST                     = 400,
    HTTP_STATUS_CODE_UNAUTHORIZED                    = 401,
    HTTP_STATUS_CODE_PAYMENT_REQUIRED                = 402,
    HTTP_STATUS_CODE_FORBIDDEN                       = 403,
    HTTP_STATUS_CODE_NOT_FOUND                       = 404,
    HTTP_STATUS_CODE_METHOD_NOT_ALLOWED              = 405,
    HTTP_STATUS_CODE_NOT_ACCEPTABLE                  = 406,
    HTTP_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED   = 407,
    HTTP_STATUS_CODE_REQUEST_TIMEOUT                 = 408,
    HTTP_STATUS_CODE_CONFLICT                        = 409,
    HTTP_STATUS_CODE_GONE                            = 410,
    HTTP_STATUS_CODE_LENGTH_REQUIRED                 = 411,
    HTTP_STATUS_CODE_PRECONDITION_FAILED             = 412,
    HTTP_STATUS_CODE_PAYLOAD_TOO_LARGE               = 413,
    HTTP_STATUS_CODE_URI_TOO_LONG                    = 414,
    HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE          = 415,
    HTTP_STATUS_CODE_RANGE_NOT_SATISFIABLE           = 416,
    HTTP_STATUS_CODE_EXPECTATION_FAILED              = 417,
    HTTP_STATUS_CODE_IM_A_TEAPOT                     = 418,
    HTTP_STATUS_CODE_MISDIRECTED_REQUEST             = 421,
    HTTP_STATUS_CODE_UNPROCESSABLE_CONTENT           = 422,
    HTTP_STATUS_CODE_LOCKED                          = 423,
    HTTP_STATUS_CODE_FAILED_DEPENDENCY               = 424,
    HTTP_STATUS_CODE_TOO_EARLY                       = 425,
    HTTP_STATUS_CODE_UPGRADE_REQUIRED                = 426,
    HTTP_STATUS_CODE_PRECONDITION_REQUIRED           = 428,
    HTTP_STATUS_CODE_TOO_MANY_REQUESTS               = 429,
    HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS   = 451,
    HTTP_STATUS_CODE_FIRST_CLIENT_ERROR              = 400,
    HTTP_STATUS_CODE_LAST_CLIENT_ERROR               = 499,

    HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR           = 500,
    HTTP_STATUS_CODE_NOT_IMPLEMENTED                 = 501,
    HTTP_STATUS_CODE_BAD_GATEWAY                     = 502,
    HTTP_STATUS_CODE_SERVICE_UNAVAILABLE             = 503,
    HTTP_STATUS_CODE_GATEWAY_TIMEOUT                 = 504,
    HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED      = 505,
    HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES         = 506,
    HTTP_STATUS_CODE_INSUFFICIENT_STORAGE            = 507,
    HTTP_STATUS_CODE_LOOP_DETECTED                   = 508,
    HTTP_STATUS_CODE_NOT_EXTENDED                    = 510,
    HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511,
    HTTP_STATUS_CODE_FIRST_SERVER_ERROR              = 500,
    HTTP_STATUS_CODE_LAST_SERVER_ERROR               = 599,
};

enum HTTPMethodKind {
    HTTP_METHOD_KIND_GET,
    HTTP_METHOD_KIND_HEAD,
    HTTP_METHOD_KIND_POST,
    HTTP_METHOD_KIND_PUT,
    HTTP_METHOD_KIND_DELETE,
    HTTP_METHOD_KIND_CONNECT,
    HTTP_METHOD_KIND_OPTIONS,
    HTTP_METHOD_KIND_TRACE,
    HTTP_METHOD_KIND_PATCH,
    HTTP_METHOD_KIND_COUNT
};


// @types______________________________________________________________________
struct HTTPRequestParams {
    u64            ID;
    HTTPMethodKind Method;
    Str8           URL;
    Str8           Body;
    Str8           UserAgent;
    Str8           Authorisation;
    Str8           ContentType;
};

struct HTTPResponse {
    u64            ID;
    u64            StartOffset;
    u64            EndOffset;
    b32            HasExtra;
    HTTPStatusCode Code;
    Str8           Body;
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________
HTTPStatusKind HTTPStatusKindFromStatusCode(HTTPStatusCode Code);
void HTTPInit(void);
void HTTPAsyncTick(void);
b32 HTTPPushRequest(OUT SafeByteRing* SafeRing, HTTPRequestParams* Params, u64 EndTimeUSecs);
b32 HTTPPopResponse(Arena* MemPool, OUT SafeByteRing* SafeRing, OUT HTTPResponse* Response, u64 EndTimeUSecs);

#endif
