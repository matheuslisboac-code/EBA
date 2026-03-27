#ifndef EBA_RETURNS_H
#define EBA_RETURNS_H

typedef enum {
    EBA_OK = 0,
    EBA_ERROR_OUT_OF_MEMORY = -1,
    EBA_ERROR_INVALID_PARAM = -2,
    EBA_ERROR_NOT_FOUND = -3,
    EBA_ERROR_CAPACITY_REACHED = -4,
} EbaResult;

#endif