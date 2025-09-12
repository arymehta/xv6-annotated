7150 // See MultiProcessor Specification Version 1.[14]
7151 
7152 struct mp {             // floating pointer
7153   uchar signature[4];           // "_MP_"
7154   void *physaddr;               // phys addr of MP config table
7155   uchar length;                 // 1
7156   uchar specrev;                // [14]
7157   uchar checksum;               // all bytes must add up to 0
7158   uchar type;                   // MP system config type
7159   uchar imcrp;
7160   uchar reserved[3];
7161 };
7162 
7163 struct mpconf {         // configuration table header
7164   uchar signature[4];           // "PCMP"
7165   ushort length;                // total table length
7166   uchar version;                // [14]
7167   uchar checksum;               // all bytes must add up to 0
7168   uchar product[20];            // product id
7169   uint *oemtable;               // OEM table pointer
7170   ushort oemlength;             // OEM table length
7171   ushort entry;                 // entry count
7172   uint *lapicaddr;              // address of local APIC
7173   ushort xlength;               // extended table length
7174   uchar xchecksum;              // extended table checksum
7175   uchar reserved;
7176 };
7177 
7178 struct mpproc {         // processor table entry
7179   uchar type;                   // entry type (0)
7180   uchar apicid;                 // local APIC id
7181   uchar version;                // local APIC verison
7182   uchar flags;                  // CPU flags
7183     #define MPBOOT 0x02           // This proc is the bootstrap processor.
7184   uchar signature[4];           // CPU signature
7185   uint feature;                 // feature flags from CPUID instruction
7186   uchar reserved[8];
7187 };
7188 
7189 struct mpioapic {       // I/O APIC table entry
7190   uchar type;                   // entry type (2)
7191   uchar apicno;                 // I/O APIC id
7192   uchar version;                // I/O APIC version
7193   uchar flags;                  // I/O APIC flags
7194   uint *addr;                  // I/O APIC address
7195 };
7196 
7197 
7198 
7199 
7200 // Table entry types
7201 #define MPPROC    0x00  // One per processor
7202 #define MPBUS     0x01  // One per bus
7203 #define MPIOAPIC  0x02  // One per I/O APIC
7204 #define MPIOINTR  0x03  // One per bus interrupt source
7205 #define MPLINTR   0x04  // One per system interrupt source
7206 
7207 
7208 
7209 
7210 
7211 
7212 
7213 
7214 
7215 
7216 
7217 
7218 
7219 
7220 
7221 
7222 
7223 
7224 
7225 
7226 
7227 
7228 
7229 
7230 
7231 
7232 
7233 
7234 
7235 
7236 
7237 
7238 
7239 
7240 
7241 
7242 
7243 
7244 
7245 
7246 
7247 
7248 
7249 
7250 // Blank page.
7251 
7252 
7253 
7254 
7255 
7256 
7257 
7258 
7259 
7260 
7261 
7262 
7263 
7264 
7265 
7266 
7267 
7268 
7269 
7270 
7271 
7272 
7273 
7274 
7275 
7276 
7277 
7278 
7279 
7280 
7281 
7282 
7283 
7284 
7285 
7286 
7287 
7288 
7289 
7290 
7291 
7292 
7293 
7294 
7295 
7296 
7297 
7298 
7299 
