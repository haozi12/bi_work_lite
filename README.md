# bi_work_lite 
**注意本文档为AI生成可能有错误**

从binary_work搬迁到此处，彻底拆分了C/C++版本，%n修改为size_t*

C版本在bi_work_lite/C++版本在bi_work_lite_cpp

## 1. 概述

`bi_work_lib_lite` 是一个轻量级二进制读写库，核心思想是**用 printf 风格格式化字符串描述二进制布局**，在**内存缓冲区**与 **FILE 文件流**之间按字段读写各种基本类型（int/float/double/short/char/字符串/原始字节等），并支持按格式串**批量反转字节序**。

库提供两套完全等价的 API：

| 版本 | 位置 | 头文件 | 符号作用域 |
|------|------|--------|------------|
| C 版 | 工作区根目录 | `bi_work.h` | 全局符号 |
| C++ 版 | `bi_work_lite_cpp/` 子目录 | `bi_work_cpp.h` | `namespace bi_work` |

两版函数签名、实现逻辑逐字等价（C++ 版仅在每个 `switch case` 外加了 `{}` 以满足 C++ 作用域规则）。均已用 MSVC（VS 2022 / VS 2026）在 `/std:c17` 与 `/std:c++17 /permissive-` 下实测**编译、链接、运行**通过。

## 2. 文件结构

```
bi_work_lib_lite/
├── bi_work.h              C 版公共头文件（声明全部 10 个函数 + 错误码枚举）
├── bfreadf.c              bfreadf      （变参包装 → vbfreadf）
├── bfwritef.c             bfwritef     （变参包装 → vbfwritef）
├── freadf.c               freadf       （变参包装 → vfreadf）
├── fwritef.c              fwritef      （变参包装 → vfwritef）
├── swap_endianf.c         swap_endianf （变参包装 → vswap_endianf）
├── vbfreadf.c             vbfreadf     （核心：按格式串从缓冲区读字段）
├── vbfwritef.c            vbfwritef    （核心：按格式串向缓冲区写字段）
├── vfreadf.c              vfreadf      （文件层：fread → vbfreadf）
├── vfwritef.c             vfwritef     （文件层：vbfwritef → fwrite）
├── vswap_endianf.c        vswap_endianf（核心：按格式串反转各字段字节序）+ static swap_bytes
└── bi_work_lite_cpp/
    ├── bi_work_cpp.h      C++ 版头文件（namespace bi_work，同上 10 个函数）
    └── *.cpp              与根目录 .c 一一对应，实现完全相同
```

## 3. 公共 API

### 3.1 错误码枚举 `ErrorCode`

| 常量 | 值 | 触发条件 |
|------|----|----------|
| `NULL_BUFFER_OR_FORMAT` | -1 | 缓冲区指针 / 格式串指针为 `NULL` |
| `NULL_FILE` | -2 | `FILE*` 为 `NULL` |
| `MALLOC_FAILURE` | -3 | 文件层临时缓冲区 `calloc` 失败 |

### 3.2 函数原型总表

| 函数 | 签名 | 所属层 |
|------|------|--------|
| `swap_endianf` | `int(const char* format, ...)` | 变参包装 |
| `vswap_endianf` | `int(const char* format, va_list args)` | 核心 |
| `bfwritef` | `int(void* dest, size_t buffer_size, const char* format, ...)` | 变参包装 |
| `bfreadf` | `int(const void* raw_buffer, size_t buffer_size, const char* format, ...)` | 变参包装 |
| `vbfwritef` | `int(void* dest, size_t buffer_size, const char* format, va_list args)` | 核心 |
| `vbfreadf` | `int(const void* raw_buffer, size_t buffer_size, const char* format, va_list args)` | 核心 |
| `fwritef` | `int(FILE* _Stream, size_t write_length, const char* format, ...)` | 变参包装 |
| `freadf` | `int(FILE* _Stream, size_t read_length, const char* format, ...)` | 变参包装 |
| `vfwritef` | `int(FILE* _Stream, size_t write_length, const char* format, va_list args)` | 文件层 |
| `vfreadf` | `int(FILE* _Stream, size_t read_length, const char* format, va_list args)` | 文件层 |

> C++ 版全部函数位于 `namespace bi_work` 内，如 `bi_work::bfwritef`。

## 4. 调用链分析

### 4.1 总览（调用关系图）

```
┌──────────── 变参包装层（... → va_list）────────────┐
│ swap_endianf ───────► vswap_endianf               │
│ bfwritef     ───────► vbfwritef                   │
│ bfreadf      ───────► vbfreadf                    │
│ fwritef      ───────► vfwritef  ──► vbfwritef     │
│ freadf       ───────► vfreadf   ──► vbfreadf      │
└───────────────────────────────────────────────────┘
                     │
        ┌────────────┴─────────────┐
        ▼                          ▼
   核心内存读写层               文件层中转
  vbfwritef / vbfreadf      vfwritef / vfreadf
  （逐字段 memmove）          （calloc 临时缓冲
                                    │ fwrite / fread
                                    ▼
                               C 标准库文件 IO）
```

### 4.2 三层架构

1. **变参包装层**（5 个 `...` 变参函数）
   - 职责：`va_start(args, format)` 将 `...` 打包为 `va_list`，转发给对应的 `v` 版本，`va_end` 收尾，返回值原样返回。
   - 典型实现（以 `bfreadf` 为例）：
     ```c
     int bfreadf(const void* raw_buffer, size_t buffer_size, const char* format, ...) {
         va_list args;
         va_start(args, format);
         int count = vbfreadf(raw_buffer, buffer_size, format, args);
         va_end(args);
         return count;
     }
     ```
   - 5 个包装函数结构完全一致，仅转发目标不同：`swap_endianf→vswap_endianf`、`bfwritef→vbfwritef`、`bfreadf→vbfreadf`、`fwritef→vfwritef`、`freadf→vfreadf`。

2. **核心内存读写层**（`vbfreadf` / `vbfwritef` / `vswap_endianf`）
   - 不接触文件、不分配内存；逐字符解析 `format`，每遇 `%` 按说明符从 `va_list` 取参数，用 `memmove` 读写字节（或 `swap_bytes` 反转字节序）。

3. **文件层**（`vfreadf` / `vfwritef`）
   - 不直接解析格式串，而是分配 `read_length/write_length` 字节临时缓冲区，把文件内容整体读入 / 把缓冲整体写出，中间委托给核心层 `vbfreadf/vbfwritef`。

### 4.3 一条完整调用链示例（文件写入）

```
用户调用 fwritef(fp, 32, "%d%d", a, b)
  → fwritef: va_start → vfwritef(fp, 32, "%d%d", args)
    → vfwritef: calloc(32) → vbfwritef(buf, 32, "%d%d", args)
        → vbfwritef: 解析 %d → memmove(buf, &a, 4)
                     解析 %d → memmove(buf+4, &b, 4)；返回 2
      → fwrite(buf, 1, 32, fp)；free(buf)；返回 2
```

### 4.4 内部静态辅助函数

- `swap_bytes(void* data, size_t size)`（定义于 `vswap_endianf.c`，`static`）
  - 用异或交换法将 `data` 指向的 `size` 个字节首尾反转（即字节序反转）。
  - `data==NULL || size==0` 时直接返回。
  - 仅为 `vswap_endianf` 服务，不对外暴露（C++ 版同样 `static`，位于 `namespace bi_work` 内）。

## 5. 核心函数实现详解

### 5.1 `vbfreadf` —— 按格式串从缓冲区读取字段

**签名**：`int vbfreadf(const void* raw_buffer, size_t buffer_size, const char* format, va_list args)`

**流程**：
1. `raw_buffer==NULL || format==NULL` → 返回 `NULL_BUFFER_OR_FORMAT(-1)`。
2. 游标 `forward_ptr` 指向缓冲区首字节，`count=0`，`ptr` 指向格式串。
3. `while (*ptr != '\0' && forward_ptr < buffer末尾)` 逐字符扫描：
   - 普通字符直接跳过（`ptr++`），只处理 `%` 开头的说明符。
   - 每个字段读取前做**边界检查**：`forward_ptr + 字段大小 > buffer末尾` 时 `goto end` 提前终止，返回已处理计数（**不会越界读**）。
4. 按说明符从 `va_list` 取出**指针参数**，`memmove` 拷贝到用户变量，游标后移字段大小，`count++`。

**各说明符读取行为**：

| 说明符 | 取参 | 动作 |
|--------|------|------|
| `%d` | `int*` | 拷贝 `sizeof(int)` 字节 |
| `%u` | `unsigned int*` | 拷贝 `sizeof(unsigned int)` |
| `%c` | `char*` | 拷贝 `sizeof(char)` |
| `%f` | `float*` | 拷贝 `sizeof(float)` |
| `%hd` / `%hu` | `short*` / `unsigned short*` | 拷贝 2 字节 |
| `%hhd` / `%hhu` | `signed char*` / `unsigned char*` | 拷贝 1 字节 |
| `%lf` | `double*` | 拷贝 `sizeof(double)` |
| `%ld` / `%lu` | `long*` / `unsigned long*` | 拷贝 `sizeof(long)` |
| `%lld` / `%llu` | `long long*` / `unsigned long long*` | 拷贝 8 字节 |
| `%s` | `char* dest, size_t size` | 拷贝 `size` 字节，然后 `dest[size-1]='\0'` **强制补终止符**（size==0 跳过不推进） |
| `%r` | `char* dest, size_t size` | 拷贝 `size` 字节，**不补终止符**（原始字节，size==0 跳过） |
| `%n` | `size_t*` | 把当前已读字节数 `(forward_ptr - buffer)` **回写**到指针；不推进游标、不计入 count |

> `%s` 与 `%r` 的区别：`%s` 保证读出的是合法 C 字符串（末尾置 0），`%r` 保留任意二进制内容。

### 5.2 `vbfwritef` —— 按格式串向缓冲区写入字段

**签名**：`int vbfwritef(void* dest, size_t buffer_size, const char* format, va_list args)`

**流程**：与 `vbfreadf` 对称。`dest==NULL || format==NULL` → 返回 -1；逐字段边界检查 + `memmove` 写入。

**关键差异（变参提升）**：由于 `...` 中实参会发生默认实参提升，写入侧必须用提升后的类型取参再强转回目标宽度：

| 说明符 | 实际取参 | 再转回 |
|--------|----------|--------|
| `%c` | `int` | `char` |
| `%f` | `double` | `float` |
| `%hd` | `int` | `short` |
| `%hu` | `unsigned int` | `unsigned short` |
| `%hhd` | `int` | `signed char` |
| `%hhu` | `unsigned int` | `unsigned char` |
| `%d`/`%u`/`%lf`/`%ld`/`%lu`/`%lld`/`%llu` | 对应类型本身 | 不变 |

**其他行为**：
- `%s` / `%r`：取 `char* source, size_t size`；`source==NULL` → `goto end`；`size==0` 跳过；写入 `size` 字节后推进游标。
- `%n`：回写**已写字节数**，不推进、不计 count。
- 边界检查同读侧：越界 `goto end`。

### 5.3 `vswap_endianf` 与 `swap_bytes` —— 按格式串批量反转字节序

**签名**：`int vswap_endianf(const char* format, va_list args)`（无缓冲区/长度参数）

**流程**：
1. `format==NULL` → 返回 `NULL_BUFFER_OR_FORMAT`。
2. 逐字符扫描格式串，每遇说明符从 `va_list` 取**变量指针**，对其指向的内存调用 `swap_bytes(ptr, 字段大小)` 做字节反转；`count++`。
3. `%s`/`%r`：取 `char* + size_t`，反转 `size` 个字节（size==0 跳过）。
4. `%n`：**不适用**（`vswap_endianf` 的 switch 中无 `case 'n'`，落入 `default` 被忽略）。
5. **无边界检查**——内存与长度由调用方保证，这是它与 vbf* 的重要区别。
6. `swap_bytes` 用异或交换首尾字节：`*s ^= *e; *e ^= *s; *s ^= *e;`，直到 `start >= end`。

### 5.4 `vfwritef` / `vfreadf` —— 文件层

**`vfwritef(FILE* _Stream, size_t write_length, const char* format, va_list args)`**：
1. `_Stream==NULL` → 返回 `NULL_FILE`。
2. `calloc(write_length, 1)` 分配临时缓冲（失败 → `perror("malloc failed")` + 返回 `MALLOC_FAILURE`）。
3. `vbfwritef(buffer, write_length, format, args)` 先把数据写入临时缓冲（缓冲初值为 0，未被写满的字节将以 0 写出）。
4. `fwrite(buffer, 1, write_length, _Stream)` 整体写出；若 `bytes < write_length` 打印 `Warning: bytes written: %zu, but expected: %zu`。
5. `free(buffer)`；**返回 vbfwritef 的 count**（成功处理的说明符个数，而非实际写出的字节数）。

**`vfreadf(FILE* _Stream, size_t read_length, const char* format, va_list args)`**：
1. `_Stream==NULL` → 返回 `NULL_FILE`。
2. `calloc(read_length, 1)` 分配（失败 → `perror` + 返回 `MALLOC_FAILURE`）。
3. `fread(buffer, 1, read_length, _Stream)` 读入；若 `bytes < read_length` 打印 `Warning: File does not have enough bytes, data might be incomplete.`。
4. `vbfreadf(buffer, bytes, format, args)`（注意：传入的是**实际读到的字节数**，不足部分由 `calloc` 置 0，格式串仍可解析）。
5. `free(buffer)`；返回 vbfreadf 的 count。

## 6. 格式说明符总表（三个 v* 核心函数共用）

> 字节数为当前平台（64 位 Windows / MSVC）实测值。"写入取参" = `vbfwritef` 从 `va_list` 取的实参（含提升）；"读取取参" = `vbfreadf` 取的指针参数。

| 格式符 | 目标类型 | 写入取参 | 读取取参 | 字节数 | swap 取参 |
|--------|----------|----------|----------|--------|-----------|
| `%d` | int | `int` | `int*` | 4 | `int*` |
| `%u` | unsigned int | `unsigned int` | `unsigned int*` | 4 | `unsigned int*` |
| `%c` | char | `int`→`char` | `char*` | 1 | `char*` |
| `%f` | float | `double`→`float` | `float*` | 4 | `float*` |
| `%lf` | double | `double` | `double*` | 8 | `double*` |
| `%hd` | short | `int`→`short` | `short*` | 2 | `short*` |
| `%hu` | unsigned short | `unsigned int`→`unsigned short` | `unsigned short*` | 2 | `unsigned short*` |
| `%hhd` | signed char | `int`→`signed char` | `signed char*` | 1 | `signed char*` |
| `%hhu` | unsigned char | `unsigned int`→`unsigned char` | `unsigned char*` | 1 | `unsigned char*` |
| `%ld` | long | `long` | `long*` | 4 | `long*` |
| `%lu` | unsigned long | `unsigned long` | `unsigned long*` | 4 | `unsigned long*` |
| `%lld` | long long | `long long` | `long long*` | 8 | `long long*` |
| `%llu` | unsigned long long | `unsigned long long` | `unsigned long long*` | 8 | `unsigned long long*` |
| `%s` | char[] | `char*, size_t` | `char*, size_t` | size | `char*, size_t` |
| `%r` | 原始字节 | `char*, size_t` | `char*, size_t` | size | `char*, size_t` |
| `%n` | size_t | `size_t*`（回写已写字节） | `size_t*`（回写已读字节） | 0 | ❌ 无此 case |

说明：
- `%s` 读取侧自动在末尾写 `'\0'`；`%r` 不写。
- `%n` 只回写字节数，不推进游标、不计入返回值。
- 未知说明符（`%` 后跟非法字符）落入 `default` 被忽略，游标继续向后扫描。

## 7. 边界检查与安全行为

| 场景 | 行为 |
|------|------|
| 缓冲区/格式串为 NULL（vbf*） | 返回 -1 |
| FILE* 为 NULL（vf*） | 返回 -2 |
| calloc 失败（vf*） | `perror("malloc failed")` + 返回 -3 |
| 字段越界（vbf*，读或写） | `goto end` 立即停止，返回已成功处理的计数（不越界访问） |
| `%s`/`%r` size==0（vbf* / vswap） | 跳过该字段，不推进 |
| 写入侧 `%s`/`%r` source==NULL | `goto end` |
| 文件数据不足（vfreadf） | stderr 打印警告；实际读取的字节传给 vbfreadf，未读到部分为 0 |
| 文件写不足（vfwritef） | stderr 打印警告 |

> 注意 `vswap_endianf` **不做任何边界/长度检查**：它直接反转调用方指针指向的内存，缓冲区大小、有效性由调用方负责。`swap_bytes` 仅对 `data==NULL || size==0` 直接返回。

## 8. 返回值语义

- 成功：返回**已成功处理的格式说明符个数**（`%n` 不计入），例如 `"%d%d"` 两个字段成功 → 返回 2。
- 失败：返回负错误码 `-1`（NULL 缓冲区/格式）、`-2`（NULL 文件）、`-3`（内存分配失败）。
- 文件层函数返回的**不是实际读写的文件字节数**，而是内部 `vbfreadf/vbfwritef` 的说明符计数；实际字节数通过 `fread/fwrite` 返回值在函数内部做完整性检查并打印警告。

## 9. C 版 与 C++ 版一致性

- 两版头文件声明的 10 个函数签名、`ErrorCode` 枚举值完全一致。
- 两版源码在**去除空白字符 + 去除 `namespace bi_work{}` 包装**后逐字符相同，实现逻辑完全一致。
- 唯一结构性差异：C++ 版每个 `switch case` 分支用 `{}` 包裹（符合 C++ 禁止"跳过带初始化变量声明"的规则），C 版不需要。
- 两个版本均已实测：MSVC `/std:c17`（C）与 `/std:c++17 /permissive-`（C++）编译零警告零错误，链接、运行同一测试用例输出完全一致。

## 10. 实测验证结果

在两套编译器（VS 2022 / MSVC 14.44、VS 2026 / MSVC 14.51）上对 C、C++ 版分别执行了同一套功能测试：

```
bfwritef count=7
bfreadf count=7  a=01020304 b=3.140000 s=hello c=100 d=987654321 e=1122334455667788 f=DEADBEEF
swap_endianf -> 04030201 (expect 04030201)
raw %r: write=1 read=1 match=1
%n: count=1 bytes=4 (expect count=1 bytes=4)
NULL/error-code checks done
file roundtrip: write=2 read=2 g1=12345678 g2=9ABCDEF0
ALL C TESTS PASSED / ALL C++ TESTS PASSED
```

覆盖点：`%d %f %s %hhd %ld %lld %u` 缓冲往返、`swap_endianf` 字节序反转、`%r` 原始拷贝、`%n` 字节数回写、NULL/错误码路径、`fwritef/freadf` 文件往返。

## 11. 已知注意事项与限制

1. **变参类型必须严格匹配**：因 `...` 存在默认实参提升，写 `%c`/`%f`/`%hd` 等时实参会被提升为 `int`/`double`，库内已做回转型转换；但读取侧必须传对应类型的指针，传错类型属未定义行为。
2. **字节序语义**：本库读写只是"逐字节拷贝"，不解释也不转换字节序；跨平台（大小端）场景下应配合 `swap_endianf` 使用。
3. **缓冲区长度必须足够**：`buffer_size` / `write_length` / `read_length` 应覆盖所有字段的总字节数，否则会在越界前提前停止（`goto end`），只处理部分字段。
4. **`%s` 的 size 语义**：`%s` 不是 C 字符串风格（不以 `\0` 决定长度），长度完全由 `size_t` 参数决定，读写两侧必须传一致的 size 才能正确往返。
5. **`%n` 不计数**：若格式串只有 `%n`，返回值为 0。
6. **文件层零填充**：`vfwritef` 缓冲区初始为 0，若 `write_length` 大于各字段总长，多出的字节会以 0 写入文件。
7. **线程安全**：函数均为无共享状态的纯函数（不使用静态可变数据），可安全并发调用（变参解析彼此独立）。

---
*本文档依据工作区全部源文件逐行分析生成，分析方法：源码通读 + 规范化文本比对 + MSVC 双版本编译/链接/运行实测。*
