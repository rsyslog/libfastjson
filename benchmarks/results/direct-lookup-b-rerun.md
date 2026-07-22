# libfastjson paired comparison

baseline ns/op divided by candidate ns/op; above 1 is faster

| Operation | Width | Position | Bytes | Speedup | MAD | >=10% | >5% regression |
|---|---:|---|---:|---:|---:|---|---|
| lookup | 8 | first | 0 | 3.0996 | 0.0359 | True | False |
| lookup | 8 | last | 0 | 2.3423 | 0.0251 | True | False |
| lookup | 8 | middle | 0 | 2.6937 | 0.0434 | True | False |
| lookup | 8 | miss | 0 | 2.7982 | 0.0349 | True | False |
| lookup | 16 | first | 0 | 3.1011 | 0.0481 | True | False |
| lookup | 16 | last | 0 | 1.9395 | 0.0310 | True | False |
| lookup | 16 | middle | 0 | 3.5117 | 0.0064 | True | False |
| lookup | 16 | miss | 0 | 1.9864 | 0.0167 | True | False |
| lookup | 32 | first | 0 | 2.3643 | 0.0255 | True | False |
| lookup | 32 | last | 0 | 2.3273 | 0.2443 | True | False |
| lookup | 32 | middle | 0 | 1.8171 | 0.0195 | True | False |
| lookup | 32 | miss | 0 | 2.0523 | 0.3216 | True | False |
| replace | 8 | first | 0 | 1.4132 | 0.1133 | True | False |
| replace | 8 | last | 0 | 1.3655 | 0.1900 | True | False |
| replace | 32 | first | 0 | 1.1913 | 0.0245 | True | False |
| replace | 32 | last | 0 | 1.5638 | 0.0297 | True | False |
