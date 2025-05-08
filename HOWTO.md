# CStruct 使用方法

## 基本的な使い方

```c
#include "cstruct.h"
#include <stdio.h>

int main() {
    uint8_t buffer[128];
    void *end;
    
    // バイナリデータへのパック
    int8_t i8 = -12;
    uint16_t u16 = 0x3456;
    float f32 = 3.14159f;
    
    // リトルエンディアンでパック
    end = cstruct_pack(buffer, sizeof(buffer), "<bhf", i8, u16, f32);
    if (end == NULL) {
        printf("パックに失敗しました\n");
        return 1;
    }
    
    // バイナリデータからのアンパック
    int8_t ri8;
    uint16_t ru16;
    float rf32;
    
    const void *ret = cstruct_unpack(buffer, sizeof(buffer), "<bhf", &ri8, &ru16, &rf32);
    if (ret == NULL) {
        printf("アンパックに失敗しました\n");
        return 1;
    }
    
    printf("アンパック結果: i8=%d, u16=0x%04X, f32=%f\n", ri8, ru16, rf32);
    
    return 0;
}
```

型指定と引数は厳密に合わせる必要があります。正しく対応が取れていないと未定義動作の原因となります。

未定義動作を回避したい場合、変数1つずつをpack/unpackする関数も使うことができます。

## 1つずつpack/unpackする

```c
#include "cstruct.h"
#include <stdio.h>

int main() {
    uint8_t buffer[128];
    uint8_t *ptr = buffer;
    
    // 個別の値をパック
    int8_t i8 = -12;
    uint16_t u16 = 0x3456;
    float f32 = 3.14159f;
    
    // リトルエンディアンでパック
    ptr = cstruct_pack_int8(ptr, i8);
    ptr = cstruct_pack_uint16_le(ptr, u16);
    ptr = cstruct_pack_float32_le(ptr, f32);
    
    // バイナリデータからのアンパック
    int8_t ri8;
    uint16_t ru16;
    float rf32;
    
    const uint8_t *rptr = buffer;
    rptr = cstruct_unpack_int8(rptr, &ri8);
    rptr = cstruct_unpack_uint16_le(rptr, &ru16);
    rptr = cstruct_unpack_float32_le(rptr, &rf32);
    
    printf("アンパック結果: i8=%d, u16=0x%04X, f32=%f\n", ri8, ru16, rf32);
    
    return 0;
}
```

## フォーマット文字列

フォーマット文字列は、パックまたはアンパックするデータの型と順序を指定します。

### エンディアン指定子

- `<`: リトルエンディアン（デフォルト）
- `>`: ビッグエンディアン

Pythonと異なり、デフォルトはリトルエンディアンで、ネイティブエンディアンは提供しません。

### 型指定子

- `b`: 8ビット符号付き整数（int8_t）
- `B`: 8ビット符号なし整数（uint8_t）
- `h`: 16ビット符号付き整数（int16_t）
- `H`: 16ビット符号なし整数（uint16_t）
- `i`: 32ビット符号付き整数（int32_t）
- `I`: 32ビット符号なし整数（uint32_t）
- `q`: 64ビット符号付き整数（int64_t）
- `Q`: 64ビット符号なし整数（uint64_t）
- `t`: 128ビット符号付き整数（int8_t[16]）
- `T`: 128ビット符号なし整数（uint8_t[16]）
- `e`: 16ビット半精度浮動小数点数（IEEE754）
- `f`: 32ビット単精度浮動小数点数（float）
- `d`: 64ビット倍精度浮動小数点数（double）
- `x`: パディング（バイト数を指定可能、例: `x4`は4バイトのパディング）

パディングしてpackした場合、該当バイトは0で埋められます。unpackの場合は該当バイトはスキップされます。

## 特殊なケース

### 半精度浮動小数点数（16ビット）

CStructは、IEEE754準拠の半精度浮動小数点数をサポートしています。これは以下の構成になっています：

- 符号部（Sign）: 1ビット
- 指数部（Exponent）: 5ビット（バイアス15）
- 仮数部（Fraction）: 10ビット

非正規化数（指数部が0で仮数部が0でない値）も適切に処理されます。

```c
// 半精度浮動小数点数の例
float f16 = 1.0f;
cstruct_pack(buffer, sizeof(buffer), "e", f16);

float rf16;
cstruct_unpack(buffer, sizeof(buffer), "e", &rf16);
```

### バッファサイズの確認

パックやアンパック時には、バッファサイズが十分かどうかを確認することが重要です。関数は、バッファサイズが不足している場合にNULLを返します。

```c
// バッファサイズチェックの例
uint8_t small_buffer[2];
void *end = cstruct_pack(small_buffer, sizeof(small_buffer), "i", 12345);
if (end == NULL) {
    printf("バッファサイズが不足しています\n");
}
```

### ポインタの取得

特定のインデックスの要素へのポインタを取得するには、`cstruct_get_ptr`関数を使用します。

```c
// ポインタ取得の例
const void *ptr = cstruct_get_ptr(buffer, sizeof(buffer), "bhf", 2);
if (ptr != NULL) {
    float value;
    cstruct_unpack_float32_le(ptr, &value);
    printf("3番目の要素（float）: %f\n", value);
}
```

#### 配列とポインタ取得

`cstruct_get_ptr`関数では、配列指定（例：`3b`）は単一のフィールドとして扱われます。つまり、配列全体が1つのインデックスとしてカウントされます。

```c
// 配列を含むフォーマット文字列でのポインタ取得
const void *ptr;

// フォーマット文字列: "b3hf" (8ビット整数、16ビット整数の配列[3要素]、32ビット浮動小数点数)
// インデックス0: 8ビット整数へのポインタ
ptr = cstruct_get_ptr(buffer, sizeof(buffer), "b3hf", 0);

// インデックス1: 16ビット整数の配列（3要素）へのポインタ
// 配列全体が1つのフィールドとしてカウントされる
ptr = cstruct_get_ptr(buffer, sizeof(buffer), "b3hf", 1);

// インデックス2: 32ビット浮動小数点数へのポインタ
ptr = cstruct_get_ptr(buffer, sizeof(buffer), "b3hf", 2);

// インデックス3以上: NULL（範囲外）
ptr = cstruct_get_ptr(buffer, sizeof(buffer), "b3hf", 3); // NULLが返される
```

配列内の特定の要素にアクセスするには、返されたポインタからオフセットを計算する必要があります。
