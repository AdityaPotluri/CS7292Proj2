#include <iostream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "pagemap/include/pagemap.h"


int main(int argc, char *argv[])
{
    // Map in a page.
	void *page = mmap(NULL, 4096, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	if (page == MAP_FAILED)
    {
		return -1;
    }
    
    // Virtual Address
    printf("VA: %p\n", page);
    
    PageTableEntry pte = PageMap::with_self().read((uint64_t)page);

    // Display the PTE.
	printf("PTE: 0x%lx\n", pte.n);
	printf("PFN: 0x%lx\n", pte.page_frame_number());
	printf("PTE.is_present: 0x%x\n", pte.is_present());
	printf("PTE.is_swaped: 0x%x\n", pte.is_swaped());
	printf("PTE.is_soft_dirty: 0x%x\n", pte.is_soft_dirty());

	return 0;
}

