/**
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************
  File:	      FLASH_SECTOR_H7.c
  Modifier:   ControllersTech.com
  Updated:    27th MAY 2021
  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com
  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.
  ***************************************************************************************************************
*/

#ifndef INC_FLASH_SECTOR_F4_H_
#define INC_FLASH_SECTOR_F4_H_

#include "stdint.h"
#include <stdbool.h>

uint32_t Flash_Write_Data (uint32_t StartSectorAddress, uint32_t *Data, uint16_t numberofwords,bool delete);

void Flash_Read_Data (uint32_t StartSectorAddress, uint32_t *RxBuf, uint16_t numberofwords);

void Convert_To_Str (uint32_t *Data, char *Buf);

uint32_t Flash_Write_NUM (uint32_t StartSectorAddress, float Num,bool delete);

float Flash_Read_NUM (uint32_t StartSectorAddress);

void EraseFlashSector(uint32_t StartSectorAddress);


#endif /* INC_FLASH_SECTOR_F4_H_ */
