# libfastjson paired comparison

baseline ns/op divided by candidate ns/op; above 1 is faster

| Operation | Width | Position | Bytes | Speedup | MAD | >=10% | >5% regression |
|---|---:|---|---:|---:|---:|---|---|
| lookup | 1 | first | 0 | 3.1259 | 0.0240 | True | False |
| lookup | 1 | miss | 0 | 2.0967 | 0.0134 | True | False |
| lookup | 4 | first | 0 | 3.1395 | 0.0252 | True | False |
| lookup | 4 | last | 0 | 3.0032 | 0.0682 | True | False |
| lookup | 4 | middle | 0 | 2.2737 | 0.0102 | True | False |
| lookup | 4 | miss | 0 | 2.1953 | 0.0200 | True | False |
| lookup | 8 | first | 0 | 3.1296 | 0.0110 | True | False |
| lookup | 8 | last | 0 | 2.3627 | 0.0214 | True | False |
| lookup | 8 | middle | 0 | 2.7175 | 0.0176 | True | False |
| lookup | 8 | miss | 0 | 2.8302 | 0.0381 | True | False |
| lookup | 16 | first | 0 | 2.9466 | 0.5195 | True | False |
| lookup | 16 | last | 0 | 1.8070 | 0.1414 | True | False |
| lookup | 16 | middle | 0 | 3.0099 | 0.1752 | True | False |
| lookup | 16 | miss | 0 | 1.7988 | 0.0718 | True | False |
| lookup | 32 | first | 0 | 1.9511 | 0.0681 | True | False |
| lookup | 32 | last | 0 | 2.1684 | 0.2031 | True | False |
| lookup | 32 | middle | 0 | 1.9115 | 0.1896 | True | False |
| lookup | 32 | miss | 0 | 1.6087 | 0.0953 | True | False |
| object | 0 | none | 0 | 1.0348 | 0.0212 | False | False |
| object | 1 | none | 0 | 0.9958 | 0.0072 | False | False |
| object | 4 | none | 0 | 1.0852 | 0.0125 | False | False |
| object | 8 | none | 0 | 1.2664 | 0.0045 | True | False |
| object | 9 | none | 0 | 1.1946 | 0.0503 | True | False |
| object | 16 | none | 0 | 1.3558 | 0.0052 | True | False |
| object | 32 | none | 0 | 1.5876 | 0.0113 | True | False |
| object-new | 0 | none | 0 | 0.9985 | 0.0176 | False | False |
| object-new | 1 | none | 0 | 0.9842 | 0.0096 | False | False |
| object-new | 4 | none | 0 | 1.0013 | 0.0114 | False | False |
| object-new | 8 | none | 0 | 0.9910 | 0.0035 | False | False |
| object-new | 9 | none | 0 | 0.9998 | 0.0042 | False | False |
| object-new | 16 | none | 0 | 0.9893 | 0.0069 | False | False |
| object-new | 32 | none | 0 | 1.0024 | 0.0023 | False | False |
| replace | 1 | first | 0 | 1.1726 | 0.0367 | True | False |
| replace | 8 | first | 0 | 1.6212 | 0.1626 | True | False |
| replace | 8 | last | 0 | 1.8613 | 0.0654 | True | False |
| replace | 32 | first | 0 | 1.6028 | 0.0207 | True | False |
| replace | 32 | last | 0 | 1.8726 | 0.0057 | True | False |
| string | 0 | none | 8 | 0.9883 | 0.0079 | False | False |
| string | 0 | none | 16 | 1.0020 | 0.0126 | False | False |
| string | 0 | none | 31 | 0.9708 | 0.0135 | False | False |
| string | 0 | none | 32 | 0.9852 | 0.0091 | False | False |
| string | 0 | none | 33 | 0.9841 | 0.0129 | False | False |
| string | 0 | none | 64 | 1.0392 | 0.0372 | False | False |
| string | 0 | none | 127 | 1.0002 | 0.0101 | False | False |
| string | 0 | none | 128 | 0.9903 | 0.0319 | False | False |
| string | 0 | none | 129 | 0.9841 | 0.0213 | False | False |
| string | 0 | none | 208 | 0.9857 | 0.0201 | False | False |
| string | 0 | none | 209 | 0.9881 | 0.0287 | False | False |
| string | 0 | none | 256 | 1.0146 | 0.0290 | False | False |
| string | 0 | none | 4096 | 1.0069 | 0.0291 | False | False |
