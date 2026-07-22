# libfastjson paired comparison

baseline ns/op divided by candidate ns/op; above 1 is faster

| Operation | Width | Position | Bytes | Speedup | MAD | >=10% | >5% regression |
|---|---:|---|---:|---:|---:|---|---|
| lookup | 1 | first | 0 | 3.1195 | 0.0427 | True | False |
| lookup | 1 | miss | 0 | 2.0522 | 0.0168 | True | False |
| lookup | 4 | first | 0 | 3.0793 | 0.0255 | True | False |
| lookup | 4 | last | 0 | 3.1707 | 0.0470 | True | False |
| lookup | 4 | middle | 0 | 2.2195 | 0.0138 | True | False |
| lookup | 4 | miss | 0 | 2.2032 | 0.0132 | True | False |
| lookup | 8 | first | 0 | 3.1352 | 0.0425 | True | False |
| lookup | 8 | last | 0 | 2.3126 | 0.0153 | True | False |
| lookup | 8 | middle | 0 | 2.6994 | 0.0423 | True | False |
| lookup | 8 | miss | 0 | 2.9226 | 0.0568 | True | False |
| lookup | 16 | first | 0 | 3.1572 | 0.0183 | True | False |
| lookup | 16 | last | 0 | 1.9043 | 0.0155 | True | False |
| lookup | 16 | middle | 0 | 3.5193 | 0.0107 | True | False |
| lookup | 16 | miss | 0 | 1.9956 | 0.0223 | True | False |
| lookup | 32 | first | 0 | 2.3998 | 0.0264 | True | False |
| lookup | 32 | last | 0 | 2.3398 | 0.0167 | True | False |
| lookup | 32 | middle | 0 | 1.8130 | 0.0053 | True | False |
| lookup | 32 | miss | 0 | 1.9420 | 0.0120 | True | False |
| object | 0 | none | 0 | 1.0227 | 0.0146 | False | False |
| object | 1 | none | 0 | 0.9764 | 0.0060 | False | False |
| object | 4 | none | 0 | 1.0829 | 0.0037 | False | False |
| object | 8 | none | 0 | 1.2736 | 0.0069 | True | False |
| object | 9 | none | 0 | 1.2007 | 0.0032 | True | False |
| object | 16 | none | 0 | 1.3524 | 0.0067 | True | False |
| object | 32 | none | 0 | 1.6026 | 0.0161 | True | False |
| object-new | 0 | none | 0 | 1.0045 | 0.0199 | False | False |
| object-new | 1 | none | 0 | 0.9758 | 0.0087 | False | False |
| object-new | 4 | none | 0 | 0.9907 | 0.0050 | False | False |
| object-new | 8 | none | 0 | 0.9889 | 0.0045 | False | False |
| object-new | 9 | none | 0 | 0.9964 | 0.0066 | False | False |
| object-new | 16 | none | 0 | 1.0031 | 0.0167 | False | False |
| object-new | 32 | none | 0 | 1.0003 | 0.0148 | False | False |
| replace | 1 | first | 0 | 1.6046 | 0.0105 | True | False |
| replace | 8 | first | 0 | 1.6165 | 0.0148 | True | False |
| replace | 8 | last | 0 | 1.8451 | 0.0203 | True | False |
| replace | 32 | first | 0 | 1.6031 | 0.0133 | True | False |
| replace | 32 | last | 0 | 1.8809 | 0.0019 | True | False |
| string | 0 | none | 8 | 0.9768 | 0.0087 | False | False |
| string | 0 | none | 16 | 1.0041 | 0.0146 | False | False |
| string | 0 | none | 31 | 0.9907 | 0.0218 | False | False |
| string | 0 | none | 32 | 0.9922 | 0.0101 | False | False |
| string | 0 | none | 33 | 0.9900 | 0.0118 | False | False |
| string | 0 | none | 64 | 0.9951 | 0.0194 | False | False |
| string | 0 | none | 127 | 0.9667 | 0.0130 | False | False |
| string | 0 | none | 128 | 0.9945 | 0.0151 | False | False |
| string | 0 | none | 129 | 0.9927 | 0.0227 | False | False |
| string | 0 | none | 208 | 0.9964 | 0.0094 | False | False |
| string | 0 | none | 209 | 0.9928 | 0.0120 | False | False |
| string | 0 | none | 256 | 1.0185 | 0.0074 | False | False |
| string | 0 | none | 4096 | 1.0018 | 0.0097 | False | False |
