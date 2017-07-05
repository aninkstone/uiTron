OUTPUT_FORMAT("elf32-h8300")
OUTPUT_ARCH("h8300h")

#include "akih8_3069.mem"

SECTIONS
{
.vectors :
  {
    head.o
    *(.vectors)
  } > VECT
  . = ALIGN(4);

.text :
  {
    __text = .; 
    entry.o(.text)
    *(.text)
    *(.rodata)
    *(.rodata.str1.1)
  } > ROM
  . = ALIGN(4);
  
  __idata_start = .; 
.data : AT(__idata_start)
  {
    __data_start = .; 
    *(.data)
  } > RAM
  . = ALIGN(2);	
  __idata_end = __idata_start + SIZEOF(.data);
    
  __bss_start = .;
.bss :
  {
    *(.bss)
    *(COMMON)
  } > RAM
  . = ALIGN(2);
  
  __bss_end = .;
.stack   :
  {
    *(.stack)
  } > STACK
}
