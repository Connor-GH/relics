#ifndef E820_REALMODE_H
#define E820_REALMODE_H
typedef struct SMAP_entry {

	uint32_t BaseL; // base address uint64_t
	uint32_t BaseH;
	uint32_t LengthL; // length uint64_t
	uint32_t LengthH;
	uint32_t Type; // entry Type
	uint32_t ACPI; // extended

}__attribute__((packed)) SMAP_entry_t;
SMAP_entry_t *getMemMap(void);
void memory_map(void);
#endif
