1700 #include "param.h"
1701 #include "types.h"
1702 #include "defs.h"
1703 #include "x86.h"
1704 #include "memlayout.h"
1705 #include "mmu.h"
1706 #include "proc.h"
1707 #include "elf.h"
1708 
1709 extern char data[];  // defined by kernel.ld
1710 pde_t *kpgdir;  // for use in scheduler()
1711 
1712 // Set up CPU's kernel segment descriptors.
1713 // Run once on entry on each CPU.
1714 void
1715 seginit(void)
1716 {
1717   struct cpu *c;
1718 
1719   // Map "logical" addresses to virtual addresses using identity map.
1720   // Cannot share a CODE descriptor for both kernel and user
1721   // because it would have to have DPL_USR, but the CPU forbids
1722   // an interrupt from CPL=0 to DPL=3.
1723   c = &cpus[cpuid()];
1724   c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
1725   c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
1726   c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
1727   c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
1728   lgdt(c->gdt, sizeof(c->gdt));
1729 }
1730 
1731 // Return the address of the PTE in page table pgdir
1732 // that corresponds to virtual address va.  If alloc!=0,
1733 // create any required page table pages.
1734 static pte_t *
1735 walkpgdir(pde_t *pgdir, const void *va, int alloc)
1736 {
1737   pde_t *pde;
1738   pte_t *pgtab;
1739 
1740   pde = &pgdir[PDX(va)];
1741   if(*pde & PTE_P){
1742     pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
1743   } else {
1744     if(alloc == 0 || (pgtab = (pte_t*)kalloc()) == 0)
1745       return 0;
1746     // Make sure all those PTE_P bits are zero.
1747     memset(pgtab, 0, PGSIZE);
1748     // The permissions here are overly generous, but they can
1749     // be further restricted by the permissions in the page table
1750     // entries, if necessary.
1751     *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
1752   }
1753   return &pgtab[PTX(va)];
1754 }
1755 
1756 // Create PTEs for virtual addresses starting at va that refer to
1757 // physical addresses starting at pa. va and size might not
1758 // be page-aligned.
1759 static int
1760 mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
1761 {
1762   char *a, *last;
1763   pte_t *pte;
1764 
1765 /* Aryan:
1766  *      walkpgdir: returns the address of entry where VA is mapped, if not present, creates the entry
1767  *      can only fail if kalloc fails()
1768  * */
1769   a = (char*)PGROUNDDOWN((uint)va);
1770   last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
1771   for(;;){
1772     if((pte = walkpgdir(pgdir, a, 1)) == 0)
1773       return -1;
1774     if(*pte & PTE_P)
1775       panic("remap");
1776     *pte = pa | perm | PTE_P;
1777     if(a == last)
1778       break;
1779     a += PGSIZE;
1780     pa += PGSIZE;
1781   }
1782     /* Aryan: If want to map multiple pages
1783      *      eg: mappages(pgdir, va, 8096, pa, perm) means two pages
1784      *      for loop:
1785      *          walkpgdir() : check if entry exists at va, if not, create it
1786      *          create mapping
1787      *          then for subsequent pages, INCREMENT a and pgsize next iteration will map a page to a+PGSIZE to next physical page
1788      */
1789   return 0;
1790 }
1791 
1792 
1793 
1794 
1795 
1796 
1797 
1798 
1799 
1800 // There is one page table per process, plus one that's used when
1801 // a CPU is not running any process (kpgdir). The kernel uses the
1802 // current process's page table during system calls and interrupts;
1803 // page protection bits prevent user code from using the kernel's
1804 // mappings.
1805 //
1806 // setupkvm() and exec() set up every page table like this:
1807 //
1808 //   0..KERNBASE: user memory (text+data+stack+heap), mapped to
1809 //                phys memory allocated by the kernel
1810 //   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
1811 //   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
1812 //                for the kernel's instructions and r/o data
1813 //   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
1814 //                                  rw data + free physical memory
1815 //   0xfe000000..0: mapped direct (devices such as ioapic)
1816 //
1817 // The kernel allocates physical memory for its heap and for user memory
1818 // between V2P(end) and the end of physical memory (PHYSTOP)
1819 // (directly addressable from end..P2V(PHYSTOP)).
1820 
1821 // This table defines the kernel's mappings, which are present in
1822 // every process's page table.
1823 static struct kmap {
1824   void *virt;
1825   uint phys_start;
1826   uint phys_end;
1827   int perm;
1828 } kmap[] = {
1829  { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
1830  { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
1831  { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, // kern data+memory
1832  { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more devices
1833 };
1834 // Aryan: data: same as the data entry in ELF headers, VA: 0x80090000..
1835 
1836 // Set up kernel part of a page table.
1837 pde_t*
1838 setupkvm(void)
1839 {
1840     /* Aryan
1841      * allocate one page from kmem.freelist and assign it to pgdir
1842      * typecast run* to char* (byte pointer), so can change individual bytes to 0
1843      * */
1844   pde_t *pgdir;
1845   struct kmap *k;
1846 
1847   if((pgdir = (pde_t*)kalloc()) == 0)
1848     return 0;
1849   memset(pgdir, 0, PGSIZE);
1850   if (P2V(PHYSTOP) > (void*)DEVSPACE)
1851     panic("PHYSTOP too high");
1852   for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
1853 
1854     /* Aryan
1855      * maps range of virtual addresses from k->virt (then accepts offset), to start addr of physical memory
1856      * */
1857     if(mappages(pgdir, k->virt, k->phys_end - k->phys_start,
1858                 (uint)k->phys_start, k->perm) < 0) {
1859       freevm(pgdir);
1860       return 0;
1861     }
1862   return pgdir;
1863 }
1864 
1865 // Allocate one page table for the machine for the kernel address
1866 // space for scheduler processes.
1867 void
1868 kvmalloc(void)
1869 {
1870   kpgdir = setupkvm();
1871   switchkvm();
1872 }
1873 
1874 // Switch h/w page table register to the kernel-only page table,
1875 // for when no process is running.
1876 void
1877 switchkvm(void)
1878 {
1879     // called when NO PROCESS IS being scheduled in the scheduler
1880   lcr3(V2P(kpgdir));   // switch to the kernel page table
1881 }
1882 
1883 // Switch TSS and h/w page table to correspond to process p.
1884 void
1885 switchuvm(struct proc *p)
1886 {
1887   if(p == 0)
1888     panic("switchuvm: no process");
1889   if(p->kstack == 0)
1890     panic("switchuvm: no kstack");
1891   if(p->pgdir == 0)
1892     panic("switchuvm: no pgdir");
1893 
1894   pushcli();
1895   mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
1896                                 sizeof(mycpu()->ts)-1, 0);
1897   mycpu()->gdt[SEG_TSS].s = 0;
1898   mycpu()->ts.ss0 = SEG_KDATA << 3;
1899   mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
1900   // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
1901   // forbids I/O instructions (e.g., inb and outb) from user space
1902   mycpu()->ts.iomb = (ushort) 0xFFFF;
1903   ltr(SEG_TSS << 3);
1904   lcr3(V2P(p->pgdir));  // switch to process's address space
1905   popcli();
1906 }
1907 
1908 // Load the initcode into address 0 of pgdir.
1909 // sz must be less than a page.
1910 void
1911 inituvm(pde_t *pgdir, char *init, uint sz)
1912 {
1913   char *mem;
1914 
1915   if(sz >= PGSIZE)
1916     panic("inituvm: more than a page");
1917   mem = kalloc();
1918   memset(mem, 0, PGSIZE);
1919   mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
1920   memmove(mem, init, sz);
1921 }
1922 
1923 // Load a program segment into pgdir.  addr must be page-aligned
1924 // and the pages from addr to addr+sz must already be mapped.
1925 int
1926 loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
1927 {
1928   uint i, pa, n;
1929   pte_t *pte;
1930 
1931   if((uint) addr % PGSIZE != 0)
1932     panic("loaduvm: addr must be page aligned");
1933   for(i = 0; i < sz; i += PGSIZE){
1934     if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
1935       panic("loaduvm: address should exist");
1936     pa = PTE_ADDR(*pte);
1937     if(sz - i < PGSIZE)
1938       n = sz - i;
1939     else
1940       n = PGSIZE;
1941     if(readi(ip, P2V(pa), offset+i, n) != n)
1942       return -1;
1943   }
1944   return 0;
1945 }
1946 
1947 
1948 
1949 
1950 // Allocate page tables and physical memory to grow process from oldsz to
1951 // newsz, which need not be page aligned.  Returns new size or 0 on error.
1952 int
1953 allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
1954 {
1955   char *mem;
1956   uint a;
1957 
1958   if(newsz >= KERNBASE)
1959     return 0;
1960   if(newsz < oldsz)
1961     return oldsz;
1962 
1963   a = PGROUNDUP(oldsz);
1964   for(; a < newsz; a += PGSIZE){
1965     mem = kalloc();
1966     if(mem == 0){
1967       cprintf("allocuvm out of memory\n");
1968       deallocuvm(pgdir, newsz, oldsz);
1969       return 0;
1970     }
1971     memset(mem, 0, PGSIZE);
1972     if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
1973       cprintf("allocuvm out of memory (2)\n");
1974       deallocuvm(pgdir, newsz, oldsz);
1975       kfree(mem);
1976       return 0;
1977     }
1978   }
1979   return newsz;
1980 }
1981 
1982 // Deallocate user pages to bring the process size from oldsz to
1983 // newsz.  oldsz and newsz need not be page-aligned, nor does newsz
1984 // need to be less than oldsz.  oldsz can be larger than the actual
1985 // process size.  Returns the new process size.
1986 int
1987 deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
1988 {
1989   pte_t *pte;
1990   uint a, pa;
1991 
1992   if(newsz >= oldsz)
1993     return oldsz;
1994 
1995   a = PGROUNDUP(newsz);
1996   for(; a  < oldsz; a += PGSIZE){
1997     pte = walkpgdir(pgdir, (char*)a, 0);
1998     if(!pte)
1999       a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
2000     else if((*pte & PTE_P) != 0){
2001       pa = PTE_ADDR(*pte);
2002       if(pa == 0)
2003         panic("kfree");
2004       char *v = P2V(pa);
2005       kfree(v);
2006       *pte = 0;
2007     }
2008   }
2009   return newsz;
2010 }
2011 
2012 // Free a page table and all the physical memory pages
2013 // in the user part.
2014 void
2015 freevm(pde_t *pgdir)
2016 {
2017   uint i;
2018 
2019   if(pgdir == 0)
2020     panic("freevm: no pgdir");
2021   deallocuvm(pgdir, KERNBASE, 0);
2022   for(i = 0; i < NPDENTRIES; i++){
2023     if(pgdir[i] & PTE_P){
2024       char * v = P2V(PTE_ADDR(pgdir[i]));
2025       kfree(v);
2026     }
2027   }
2028   kfree((char*)pgdir);
2029 }
2030 
2031 // Clear PTE_U on a page. Used to create an inaccessible
2032 // page beneath the user stack.
2033 void
2034 clearpteu(pde_t *pgdir, char *uva)
2035 {
2036   pte_t *pte;
2037 
2038   pte = walkpgdir(pgdir, uva, 0);
2039   if(pte == 0)
2040     panic("clearpteu");
2041   *pte &= ~PTE_U;
2042 }
2043 
2044 
2045 
2046 
2047 
2048 
2049 
2050 // Given a parent process's page table, create a copy
2051 // of it for a child.
2052 pde_t*
2053 copyuvm(pde_t *pgdir, uint sz)
2054 {
2055   pde_t *d;
2056   pte_t *pte;
2057   uint pa, i, flags;
2058   char *mem;
2059 
2060   if((d = setupkvm()) == 0)
2061     return 0;
2062   for(i = 0; i < sz; i += PGSIZE){
2063     if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
2064       panic("copyuvm: pte should exist");
2065     if(!(*pte & PTE_P))
2066       panic("copyuvm: page not present");
2067     pa = PTE_ADDR(*pte);
2068     flags = PTE_FLAGS(*pte);
2069     if((mem = kalloc()) == 0)
2070       goto bad;
2071     memmove(mem, (char*)P2V(pa), PGSIZE);
2072     if(mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
2073       kfree(mem);
2074       goto bad;
2075     }
2076   }
2077   return d;
2078 
2079 bad:
2080   freevm(d);
2081   return 0;
2082 }
2083 
2084 
2085 
2086 
2087 
2088 
2089 
2090 
2091 
2092 
2093 
2094 
2095 
2096 
2097 
2098 
2099 
2100 // Map user virtual address to kernel address.
2101 char*
2102 uva2ka(pde_t *pgdir, char *uva)
2103 {
2104   pte_t *pte;
2105 
2106   pte = walkpgdir(pgdir, uva, 0);
2107   if((*pte & PTE_P) == 0)
2108     return 0;
2109   if((*pte & PTE_U) == 0)
2110     return 0;
2111   return (char*)P2V(PTE_ADDR(*pte));
2112 }
2113 
2114 // Copy len bytes from p to user address va in page table pgdir.
2115 // Most useful when pgdir is not the current page table.
2116 // uva2ka ensures this only works for PTE_U pages.
2117 int
2118 copyout(pde_t *pgdir, uint va, void *p, uint len)
2119 {
2120   char *buf, *pa0;
2121   uint n, va0;
2122 
2123   buf = (char*)p;
2124   while(len > 0){
2125     va0 = (uint)PGROUNDDOWN(va);
2126     pa0 = uva2ka(pgdir, (char*)va0);
2127     if(pa0 == 0)
2128       return -1;
2129     n = PGSIZE - (va - va0);
2130     if(n > len)
2131       n = len;
2132     memmove(pa0 + (va - va0), buf, n);
2133     len -= n;
2134     buf += n;
2135     va = va0 + PGSIZE;
2136   }
2137   return 0;
2138 }
2139 
2140 
2141 
2142 
2143 
2144 
2145 
2146 
2147 
2148 
2149 
2150 // Blank page.
2151 
2152 
2153 
2154 
2155 
2156 
2157 
2158 
2159 
2160 
2161 
2162 
2163 
2164 
2165 
2166 
2167 
2168 
2169 
2170 
2171 
2172 
2173 
2174 
2175 
2176 
2177 
2178 
2179 
2180 
2181 
2182 
2183 
2184 
2185 
2186 
2187 
2188 
2189 
2190 
2191 
2192 
2193 
2194 
2195 
2196 
2197 
2198 
2199 
2200 // Blank page.
2201 
2202 
2203 
2204 
2205 
2206 
2207 
2208 
2209 
2210 
2211 
2212 
2213 
2214 
2215 
2216 
2217 
2218 
2219 
2220 
2221 
2222 
2223 
2224 
2225 
2226 
2227 
2228 
2229 
2230 
2231 
2232 
2233 
2234 
2235 
2236 
2237 
2238 
2239 
2240 
2241 
2242 
2243 
2244 
2245 
2246 
2247 
2248 
2249 
2250 // Blank page.
2251 
2252 
2253 
2254 
2255 
2256 
2257 
2258 
2259 
2260 
2261 
2262 
2263 
2264 
2265 
2266 
2267 
2268 
2269 
2270 
2271 
2272 
2273 
2274 
2275 
2276 
2277 
2278 
2279 
2280 
2281 
2282 
2283 
2284 
2285 
2286 
2287 
2288 
2289 
2290 
2291 
2292 
2293 
2294 
2295 
2296 
2297 
2298 
2299 
