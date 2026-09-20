#include "nc_http.h"

HTTPStatusKind 
HTTPStatusKindFromStatusCode(HTTPStatusCode Code)
{
    HTTPStatusKind Result = HTTP_STATUS_KIND_NULL;

    if (
        Code >= HTTP_STATUS_CODE_FIRST_INFORMATIONAL &&
        Code <= HTTP_STATUS_CODE_LAST_INFORMATIONAL
    ) {
        Result = HTTP_STATUS_KIND_INFO;
    } else if (
        Code >= HTTP_STATUS_CODE_FIRST_SUCCESSFUL &&
        Code <= HTTP_STATUS_CODE_LAST_SUCCESSFUL
    ) {
        Result = HTTP_STATUS_KIND_SUCCESS;
    } else if (
        Code >= HTTP_STATUS_CODE_FIRST_REDIRECTION &&
        Code <= HTTP_STATUS_CODE_LAST_REDIRECTION
    ) {
        Result = HTTP_STATUS_KIND_REDIRECT;
    } else if (
        Code >= HTTP_STATUS_CODE_FIRST_CLIENT_ERROR &&
        Code <= HTTP_STATUS_CODE_LAST_CLIENT_ERROR
    ) {
        Result = HTTP_STATUS_KIND_CLIENT_ERROR;
    } else if (
        Code >= HTTP_STATUS_CODE_FIRST_SERVER_ERROR &&
        Code <= HTTP_STATUS_CODE_LAST_SERVER_ERROR
    ) {
        Result = HTTP_STATUS_KIND_SERVER_ERROR;
    }

    return Result;
}
