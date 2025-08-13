#ifndef GDT_H
#define GDT_H

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char granularity);
void gdt_install();

#endif
