/**
  **************************************************************************
  * @file     common.h
  * @brief    common header file
  **************************************************************************
  */
#ifndef _COMMON_H
#define _COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern void Int2Str(uint8_t* str,int32_t intnum);
extern int32_t Str2UInt(uint8_t *inputstr, uint32_t *uintnum);
#endif  /* _COMMON_H */
