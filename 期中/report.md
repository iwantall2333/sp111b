研究程式 : [semu.c](https://github.com/riscv2os/semu/blob/master/ccc/semu.c)

底下的內文的式碼來自[semu.c](https://github.com/riscv2os/semu/blob/master/ccc/semu.c)，是由chatgpt生成，
而我則是對chatgpt的回答做排版、畫重點( 粗體字即是重點，只是github會看不太出來**粗體字**的部分 )，
這也是我對[semu.c](https://github.com/riscv2os/semu/blob/master/ccc/semu.c)作的筆記

---

# 前言

### 虛擬機

虛擬機的目的是提供一個隔離的執行環境，使不同的作業系統和應用程序能夠在同一台主機上同時運行，而互不干擾。這種隔離性可以提供更高的系統利用率、更好的資源管理和更好的安全性。

底下的程式碼中，它是一個模擬了一個虛擬機器的環境。程式碼中定義了虛擬機器的不同部分，如**記憶體**、**中斷控制器**、**磁碟控制介面**等的基址和大小。

這些定義可以讓我們在虛擬機器上進行相應的操作和訪問，例如讀寫記憶體、處理中斷、訪問磁碟等。這個程式碼是一個模擬虛擬機器的範例，用於展示虛擬機器的結構和基本操作。

---
---

# 檔案引用

### .h檔

```c
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
```

##### pthread.h：

說明：**提供了使用 POSIX 線程（POSIX threads）的函式庫**。
<details>
  <summary>POSIX介紹</summary>

>POSIX（Portable Operating System Interface）線程是一個**用於多執行緒編程的標準接口**。它定義了一套函式、常數和資料類型，用於創建、管理和同步多個執行緒。
>
>POSIX線程的主要目的是提供一種可移植的方法來處理多執行緒編程，使得程式碼可以在不同的作業系統上運行而不需要進行太多的修改。POSIX線程提供了一套標準的API，讓**開發人員能夠以相似的方式創建和管理執行緒，並在這些執行緒之間進行同步和通訊。**
>
>使用POSIX線程，您可以執行以下操作：
>
>- **創建**新的執行緒：使用pthread_create函式創建新的執行緒，並指定執行的函式。
>- **管理**執行緒的屬性：使用pthread_attr_*系列函式設置和獲取執行緒的屬性，例如優先級、堆疊大小等。
>- **等待執行緒的結束**：使用pthread_join函式等待一個執行緒的結束，並獲取其返回值。
>- 執行緒**同步**：使用互斥鎖（mutex）、條件變量（condition variable）和信號量（semaphore）等機制實現執行緒之間的同步和互斥。
>- **取消**執行緒：使用pthread_cancel函式取消一個執行緒的運行。
>- **處理執行緒間的資料共享**：使用互斥鎖和條件變量等機制保護共享資源，以防止多個執行緒同時訪問導致競爭條件。
>
>POSIX線程是一個非常強大和常用的多執行緒編程工具，它可以在不同的作業系統上提供一致的多執行緒支持，使得開發人員能夠編寫可移植且高效的多執行緒程式碼。
</details>
</br>

功能：定義了線程相關的函式、常數和資料類型，用於**創建**、**管理**和**同步多個執行緒**。

##### unistd.h：

說明：提供了**POSIX作業系統API**的函式。
功能：**定義了許多POSIX系統調用**（system call），**包括文件和目錄操作、進程控制、檔案描述符操作等。**


##### stdbool.h：

說明：引入了一種新的資料型別 bool，用於表示真（true）或假（false）的值。
功能：定義了 bool 型別和 true、false 的宏定義，以及與布爾邏輯運算相關的常數和函式。
##### stdint.h：

說明：提供了固定寬度的整數型別的定義。
功能：定義了不同位元數的整數型別，如 int8_t、uint16_t 等，確保它們在不同系統上具有相同的位元數。
##### stdio.h：

說明：提供了標準輸入輸出（I/O）相關的函式。
功能：**定義了輸入輸出函式，如 printf、scanf、fopen、fclose 等**，以及相關的資料型別和宏定義。
##### stdlib.h：

說明：**提供常用的函式庫函式**。
功能：定義了**內存分配函式**、**數學**函式、**隨機數**生成函式、**字串轉換**函式等。

##### string.h：

說明：提供了字串處理相關的函式。
功能：定義了字串操作函式，如**字串複製、連接、比較、搜索等**，以及相關的資料型別和宏定義。

---
---

# 定義

### RAM 定義

```c
//定義記憶體大小，xv6 使用的記憶體大小是 128MiB
#define RAM_SIZE (1024 * 1024 * 128)
//與 QEMU 虛擬機器相同，記憶體從 0x80000000 開始。
#define RAM_BASE 0x80000000
```
---

### 局部中斷定義

**Core Local Interruptor (CLINT)** 是一個在 RISC-V 架構中常見的硬體元件，用於處理局部中斷。

```c
#define CLINT_BASE 0x2000000
#define CLINT_SIZE 0x10000
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000)
#define CLINT_MTIME (CLINT_BASE + 0xbff8)
```

##### CLINT_BASE：
表示 CLINT 的基礎位址，被設置為`0x2000000`。**CLINT 的起始位址，用於存儲 CLINT 相關的資訊和寄存器。**

##### CLINT_SIZE：
表示 CLINT 的大小，被設置為`0x10000`。**這是 CLINT 的範圍大小，指示 CLINT 佔用的記憶體空間大小。**

##### CLINT_MTIMECMP：
表示**CLINT 的 MTIMECMP 寄存器的位址。**
MTIMECMP 是 CLINT 中的計時器比較寄存器，**用於設置定時中斷的觸發時間**。
`CLINT_BASE + 0x4000` 表示 MTIMECMP 寄存器相對於 CLINT 基礎位址的偏移量。


##### CLINT_MTIME：
表示 **CLINT 的 MTIME 寄存器的位址。**
MTIME 是 CLINT 中的計時器寄存器，**用於存儲系統的運行時間**。
`CLINT_BASE + 0xbff8`表示 MTIME 寄存器相對於 CLINT 基礎位址的偏移量。

---

### 全局中斷定義

**Platform-Level Interrupt Controller (PLIC)** 是一個在 RISC-V 架構中常見的硬體元件，用於處理全局中斷（外部中斷）。

```c
#define PLIC_BASE 0xc000000
#define PLIC_SIZE 0x4000000
#define PLIC_PENDING (PLIC_BASE + 0x1000)
#define PLIC_SENABLE (PLIC_BASE + 0x2080)
#define PLIC_SPRIORITY (PLIC_BASE + 0x201000)
#define PLIC_SCLAIM (PLIC_BASE + 0x201004)
```

##### PLIC_BASE：
表示 PLIC 的基礎位址，被設置為 0xc000000。這是 PLIC 的起始位址，用於存儲 PLIC 相關的資訊和寄存器。

##### PLIC_SIZE：
表示 PLIC 的大小，被設置為 0x4000000。這是 PLIC 的範圍大小，指示 PLIC 佔用的記憶體空間大小。

##### PLIC_PENDING：
表示 **PLIC 的 PENDING 寄存器的位址**。PLIC_BASE + 0x1000 表示 PENDING 寄存器相對於 PLIC 基礎位址的偏移量。**PENDING 寄存器用於標記等待處理的中斷**。

##### PLIC_SENABLE：
表示 PLIC 的 SENABLE 寄存器的位址。PLIC_BASE + 0x2080 表示 SENABLE 寄存器相對於 PLIC 基礎位址的偏移量。**SENABLE 寄存器用於設置使能或禁用特定中斷**。

##### PLIC_SPRIORITY：
表示 PLIC 的 SPRIORITY 寄存器的位址。PLIC_BASE + 0x201000 表示 SPRIORITY 寄存器相對於 PLIC 基礎位址的偏移量。**SPRIORITY 寄存器用於設置中斷的優先級。**

##### PLIC_SCLAIM：
表示 PLIC 的 SCLAIM 寄存器的位址。PLIC_BASE + 0x201004 表示 SCLAIM 寄存器相對於 PLIC 基礎位址的偏移量。**SCLAIM 寄存器用於聲明處理完畢的中斷。**

---

### 非同步收發傳輸器定義

UART (Universal Asynchronous Receiver-Transmitter)，非同步收發傳輸器 是一個常見的**串列通訊**介面，用於透過串行**連接/傳送/接收資料**。
```c
// UART (透過 UART 傳送/接收 終端機 的訊息)
#define UART_BASE 0x10000000
#define UART_SIZE 0x100
enum { UART_RHR = UART_BASE, UART_THR = UART_BASE };
enum { UART_LCR = UART_BASE + 3, UART_LSR = UART_BASE + 5 };
enum { UART_LSR_RX = 1, UART_LSR_TX = 1 << 5 };
```

##### UART_BASE：
表示 UART 的基礎位址，被設置為 0x10000000。這是 **UART 控制器的起始位址**，用於存儲 UART 相關的寄存器和緩衝區。

##### UART_SIZE：
表示 UART 的大小，被設置為 0x100。這是 **UART 控制器的大小**，指示 UART 佔用的記憶體空間大小。

##### UART_RHR：
表示 UART 的**接收器**暫存器（Receive Holding Register）的位址，被設置為 UART_BASE。**接收器暫存器用於存儲從 UART 接收到的資料**。

##### UART_THR：
表示 UART 的**傳送器**暫存器（Transmit Holding Register）的位址，被設置為 UART_BASE。傳送器暫存器**用於將資料傳送到 UART**。

##### UART_LCR：
表示 UART 的**線路控制暫存器**（Line Control Register）的位址，被設置為 UART_BASE + 3。線路控制寄存器**用於設置 UART 的傳輸格式和通訊參數**。

##### UART_LSR：
表示 UART 的**線路狀態寄存器**（Line Status Register）的位址，被設置為 UART_BASE + 5。線路狀態寄存器**提供有關 UART 的狀態資訊**，**如接收緩衝區是否有資料可讀或傳送緩衝區是否準備好接收資料等**。

##### UART_LSR_RX：
表示 UART 線路狀態寄存器 **(UART_LSR) 的接收位元（Receive）的位元遮罩**，被設置為 1。該位元遮罩**用於檢查接收狀態**。

<details>
  <summary>UART_LSR_RX介紹</summary>
當我們設置 UART_LSR_RX 的位元遮罩為 1，它用於檢查 UART 線路狀態寄存器 (UART_LSR) 中的接收位元。UART_LSR 是一個位於 UART 控制器中的寄存器，提供有關 UART 的狀態資訊。

**當 UART 接收到資料時，它會將資料存儲在接收緩衝區中。這時，UART_LSR 的接收位元會被設置為 1，表示接收緩衝區中有資料可供讀取。**

我們可以通過檢查 UART_LSR 的接收位元狀態，來確認是否有資料可供讀取。**當 UART_LSR_RX 的位元遮罩與 UART_LSR 寄存器進行位元運算時，如果接收位元為 1，運算結果將不為零，表示接收緩衝區中有資料可供讀取**。
反之，如果接收位元為 0，運算結果將為零，表示接收緩衝區是空的，沒有資料可供讀取。

因此，通過使用 UART_LSR_RX 位元遮罩，我們可以輕鬆地檢查 UART 的接收狀態，以確定是否有資料可供讀取。這對於進行 UART 通訊時的資料接收操作非常有用。

</details>

##### UART_LSR_TX：
表示 UART 線路狀態寄存器 **(UART_LSR)的傳送位元（Transmit）的位元遮罩**，被設置為 1 << 5。該位元遮罩**用於檢查傳送狀態**。

---

### 磁碟控制介面定義

當虛擬機器需要**與主機中的裝置進行通信**時，VIRTIO（磁碟控制介面） 提供了一個標準化的介面和協議，**使虛擬機器中的驅動程式能夠與裝置進行通信，而不需要了解具體的裝置細節**。

```c
#define VIRTIO_BASE 0x10001000  //VIRTIO 控制器的基礎位址
#define VIRTIO_SIZE 0x1000      //VIRTIO 控制器的大小
#define VIRTIO_MAGIC (VIRTIO_BASE + 0x000)  //VIRTIO 魔數的位址
#define VIRTIO_VERSION (VIRTIO_BASE + 0x004)  //VIRTIO 版本號的位址
#define VIRTIO_DEVICE_ID (VIRTIO_BASE + 0x008)  //VIRTIO 設備 ID 的位址
#define VIRTIO_VENDOR_ID (VIRTIO_BASE + 0x00c)  //VIRTIO 廠商 ID 的位址
#define VIRTIO_DEVICE_FEATURES (VIRTIO_BASE + 0x010)  //VIRTIO 設備功能的位址
#define VIRTIO_DRIVER_FEATURES (VIRTIO_BASE + 0x020)  //VIRTIO 驅動程式功能的位址
#define VIRTIO_GUEST_PAGE_SIZE (VIRTIO_BASE + 0x028)  //VIRTIO 客戶端頁面大小的位址
#define VIRTIO_QUEUE_SEL (VIRTIO_BASE + 0x030)  
#define VIRTIO_QUEUE_NUM_MAX (VIRTIO_BASE + 0x034)
#define VIRTIO_QUEUE_NUM (VIRTIO_BASE + 0x038)
#define VIRTIO_QUEUE_PFN (VIRTIO_BASE + 0x040)
#define VIRTIO_QUEUE_NOTIFY (VIRTIO_BASE + 0x050)
#define VIRTIO_STATUS (VIRTIO_BASE + 0x070)

enum { VIRTIO_VRING_DESC_SIZE = 16, VIRTIO_DESC_NUM = 8 };
```

##### VIRTIO_MAGIC：
VIRTIO 魔數的位址，用於**識別 VIRTIO 設備**。

##### VIRTIO_VERSION：
VIRTIO 版本號的位址，用於**指示 VIRTIO 的版本**。

##### VIRTIO_DEVICE_ID：
VIRTIO 設備 ID 的位址，用於**標識特定的 VIRTIO 設備**。

##### VIRTIO_VENDOR_ID：
VIRTIO 廠商 ID 的位址，用於**標識製造 VIRTIO 設備的廠商**。

##### VIRTIO_DEVICE_FEATURES：
VIRTIO 設備功能的位址，用於**指示 VIRTIO 設備支援的功能**。

##### VIRTIO_DRIVER_FEATURES：
VIRTIO 驅動程式功能的位址，用於**指示驅動程式支援的功能**。

##### VIRTIO_GUEST_PAGE_SIZE：
VIRTIO 客戶端頁面大小的位址，用於**指定客戶端使用的頁面大小**。

##### VIRTIO_QUEUE_SEL：
**VIRTIO 隊列選擇器的位址**，**用於選擇要操作的 VIRTIO 隊列。**

##### VIRTIO_QUEUE_NUM_MAX：
**VIRTIO 隊列最大數量**的位址，用於指示支援的最大隊列數量。

##### VIRTIO_QUEUE_NUM：
**VIRTIO 隊列數量**的位址，用於指示實際使用的隊列數量。

##### VIRTIO_QUEUE_PFN：

**VIRTIO 隊列物理頁框號碼**的位址，用於指定隊列的物理頁框號碼。

##### VIRTIO_QUEUE_NOTIFY：
VIRTIO 隊列通知的位址，**用於通知設備有新的隊列項目需要處理**。

##### VIRTIO_STATUS：
VIRTIO 設備狀態的位址，用**於指示 VIRTIO 設備的狀態**。

此外，`VIRTIO_VRING_DESC_SIZE` 和 `VIRTIO_DESC_NUM` 是枚舉常數，用於**指定 VIRTIO 隊列描述符的大小和數量**。

#### VIRTIO 的主要功能和運作方式：

1. 虛擬化框架：VIRTIO 提供了一個通用的虛擬化框架，用於在虛擬機器和主機之間建立通信通道。這使得虛擬機器能夠模擬各種不同的裝置，如網絡介面卡、磁碟驅動器、串口等。

2. 驅動程式介面：VIRTIO 定義了一組標準的驅動程式介面，包括配置設定、數據傳輸和設備管理等功能。這些介面允許虛擬機器中的驅動程式與虛擬化層進行通信，以設置和操作裝置。

3. 虛擬化設備：VIRTIO 提供了一個通用的虛擬化設備框架，該框架定義了一組共享的設備資源和協議，以實現虛擬機器與裝置之間的通信和數據傳輸。這些設備可以模擬各種不同的裝置，並提供設備特定的功能和特性。

4. 資源分配和配置：VIRTIO 允許驅動程式為虛擬機器配置和分配必要的資源，包括記憶體、中斷和 I/O 位址等。這些資源用於設備和驅動程式之間的通信和數據傳輸。

5. 非同步通信：**VIRTIO 使用隊列機制實現虛擬機器和裝置之間的非同步通信**。每個 VIRTIO 設備都有一個或多個隊列，其中包含描述符（Descriptor）和可用/使用的環（Ring）。
    **虛擬機器的驅動程式將要傳輸的數據描述符放入隊列中，裝置在完成相應的操作後通知驅動程式。這種非同步通信方式可以提高效能和吞吐量。**

總之，VIRTIO 提供了一個標準的虛擬化框架和通用的驅動程式介面，使虛擬機器能夠與主機中的裝置進行高效的通信和數據傳輸。這使得驅動程式開發更加簡化且可攜帶性更強，同時提供了良好的性能和互操作性。

---
---

# 主要程式碼

### 範圍檢查函式

```c
/* Range check
 * For any variable range checking:
 *     if (x >= minx && x <= maxx) ...
 * it is faster to use bit operation:
 *     if ((signed)((x - minx) | (maxx - x)) >= 0) ...
 */
#define RANGE_CHECK(x, minx, size) \
    ((int32_t)((x - minx) | (minx + size - 1 - x)) >= 0)
```
該巨集接受三個參數：`x`、`minx` 和 `size`。它的目的是檢查變數 `x` 是否在指定的範圍 `[minx, minx + size - 1]` 內。

具體來說，這個巨集使用了位操作的技巧，通過計算 `(x - minx) | (minx + size - 1 - x)` 的結果來判斷 x 是否在範圍內。如果結果大於等於 0，則表示 x 在範圍內，返回 true（非零值）；否則，表示 x 不在範圍內，返回 false（0）。

<details>
  <summary>程式碼詳解</summary>

1. `(x - minx) | (minx + size - 1 - x)`: 這部分計算兩個位元遮罩，其中一個位元遮罩 (x - minx) **確保 x 大於等於 minx**，另一個位元遮罩 (minx + size - 1 - x) **確保 x 小於等於 `minx + size - 1`**。這兩個位元遮罩使用位元運算符 `|`（位元 OR）結合在一起。

2. `int32_t`：這部分將結果轉換為 int32_t 型別，以便進行比較。

3. `((x - minx) | (minx + size - 1 - x)) >= 0`：這是整個巨集的比較運算式。如果 (x - minx) | (minx + size - 1 - x) 的結果大於等於 0，則表示 x 在指定範圍內，返回 true（非零值）；否則，表示 x 不在範圍內，返回 false（0）。
</details>

---

### Machine level 與 Supervisor level 的定義

#### 機器層級（Machine level）和監管者層級（Supervisor level）

在 RISC-V 架構中，機器層級（Machine level）和監管者層級（Supervisor level）是特定的特權模式，用於區分不同的特權級別和操作模式。

1. 機器層級（Machine level）：

    - 機器層級是 RISC-V 架構中最高的特權級別，也被稱為 M 模式。
    - 在機器層級，所有的系統資源和控制權都可以被訪問和操作，包括訪問機器層級的控制狀態寄存器（CSRs）和系統資源。
    - 它是操作系統的最高特權模式，用於管理整個系統的運行，如中斷處理、例外處理、記憶體管理等。
    - 在機器層級，可以進行特權指令的執行，設置和控制其他特權級別（如監管者層級）的運行環境。

2. 監管者層級（Supervisor level）：

    - 監管者層級是 RISC-V 架構中的一個特權級別，也被稱為 S 模式。
    - 在監管者層級，有限的特權操作和資源可以被訪問和操作，但相對於機器層級，限制更多。
    - 它是用於操作系統的主要特權模式，用於管理操作系統的運行，如進程管理、資源分配、中斷處理等。
    - 在監管者層級，可以訪問和操作監管者層級的控制狀態寄存器（CSRs）和一部分系統資源，但不能訪問機器層級的資源和控制權。

總結來說，機器層級是最高的特權級別，用於管理整個系統的運行，而監管者層級是操作系統的主要特權級別，用於管理操作系統的運行。它們在特權級別、訪問權限和操作範圍上有所不同。

#### 機器層級（Machine level）控制狀態寄存器的定義

在 RISC-V 架構中，**控制狀態寄存器**（Control and Status Registers, **CSRs**）是用於控制處理器行為和保存系統狀態的特殊寄存器。

```c
/* Machine level CSRs */
enum { MSTATUS = 0x300, MEDELEG = 0x302, MIDELEG, MIE, MTVEC };
enum { MEPC = 0x341, MCAUSE, MTVAL, MIP };
```

這段程式碼**定義了一些機器層次（Machine level）的控制狀態寄存器**（Control and Status Registers, CSRs）。

以下是每個定義的 CSR 的說明：

##### MSTATUS (0x300)：
機器狀態寄存器，用於**控制和保存機器的執行狀態**，**例如中斷使能位、特權級別等**。

##### MEDELEG (0x302)：
機器**例外委派寄存器**，用於**指示哪些例外由機器直接處理，而不委派給特權模式**。

##### MIDELEG：
機器**中斷委派寄存器**，與 MEDELEG 類似，但是用於中斷委派。

##### MIE：
機器**中斷使能寄存器**，**用於控制各種中斷的使能**。

##### MTVEC：
機器中斷向量基址**寄存器**，**指定中斷處理程序的基址**。

##### MEPC (0x341)：
機器例外指令計數器寄存器，**保存例外指令的地址，即例外發生時 CPU 正在執行的指令的地址**。

##### MCAUSE：
機器例外原因寄存器，**保存導致例外的原因，例如中斷號碼或異常原因碼**。

##### MTVAL：
機器例外值寄存器，用於**保存導致例外的相關值或引數**，**例如存儲非法指令的指令碼**。

##### MIP：
機器**中斷請求寄存器**，用於**表示各種中斷的請求狀態**。

這些 CSRs 在 RISC-V 系統中起著關鍵作用，**用於控制和監視處理器的執行，處理中斷和例外，以及保存相關狀態和值**。它們對於操作系統和處理器設計者來說都是重要的元件。

#### 監管者層級（Supervisor level）控制狀態寄存器的定義

```c
/* Supervisor level CSRs */
enum { SSTATUS = 0x100, SIE = 0x104, STVEC };
enum { SEPC = 0x141, SCAUSE, STVAL, SIP, SATP = 0x180 };
```
這段程式碼定義了一組監管者層級（Supervisor level）的控制狀態寄存器（CSR）。

具體來說，它列出了一些常見的監管者層級的 CSR，以及它們對應的位址:

##### SSTATUS（0x100）：
監管者狀態寄存器，**用於存儲監管者模式的系統狀態和設定**。

##### SIE（0x104）：
監管者**中斷使能寄存器**，用於**控制監管者模式下的中斷使能**。

##### STVEC：
監管者**中斷向量表基址寄存**器，用於**指示**監管者模式下的**中斷處理程序的位置**。

##### SEPC（0x141）：
監管者**例外程序計數器**，存儲**例外處理程序的位置**。

##### SCAUSE：
監管者例外原因寄存器，**指示觸發例外的原因**。

##### STVAL：
監管者例外值寄存器，用於**存儲造成例外的相關數值**。

##### SIP：
監管者中斷請求寄存器，**指示**監管者模式下的**中斷請求狀態**。

##### SATP（0x180）：
**監管者地址轉換和保護寄存器**，**用於管理頁表和虛擬地址轉換**。

這些 CSR 提供了操作系統或監管者層級軟體進行系統控制、中斷處理、例外處理和地址轉換等功能的介面。**操作系統或監管者軟體可以讀取和寫入這些寄存器，以配置和管理系統的行為和狀態**。

#### MIP(中斷請求寄存器)細部定義

這段程式碼定義了一組監管者中斷優先級寄存器（MIP）中的位元遮罩:

```c
enum { MIP_SSIP = 1ULL << 1, MIP_MSIP = 1ULL << 3, MIP_STIP = 1ULL << 5 };
enum { MIP_MTIP = 1ULL << 7, MIP_SEIP = 1ULL << 9, MIP_MEIP = 1ULL << 11 };
```

<details>
  <summary>1ULL介紹</summary>

>1ULL 是一個表示無符號 64 位整數（unsigned long long）的數值常數。在這個上下文中，ULL 是後綴，表示數值的類型為無符號長整數。
>
>使用 **1ULL 的目的是表示一個只有最低位元為 1 的 64 位無符號整數。這在位元操作中常見，用於設置或檢查特定的位元。**
>
>以 1ULL << 1 為例，它表示將 1ULL 的位元往左移動 1 個位元，相當於將最低位元的 1 移動到次低位元位置，結果為 2ULL。同樣地，1ULL << 3 表示將最低位元的 1 移動到第 3 個位元位置，結果為 8ULL。
>
>因此，1ULL 可以用於創建只有一個位元為 1，其他位元都為 0 的數值，這在位元操作和位元運算中非常有用。
</details>

具體來說，它定義了以下位元遮罩：

##### MIP_SSIP：
代表監管者中斷優先級寄存器中的「監管者軟體中斷請求」位元（Supervisor Software Interrupt Pending）。這個位元表示**監管者模式下的軟體中斷請求是否被觸發**。

##### MIP_MSIP：
代表監管者中斷優先級寄存器中的「機器軟體中斷請求」位元（Machine Software Interrupt Pending）。這個位元表示**機器模式下的軟體中斷請求是否被觸發**。

##### MIP_STIP：
代表監管者中斷優先級寄存器中的「監管者計時器中斷請求」位元（Supervisor Timer Interrupt Pending）。這個位元表示**監管者模式下的計時器中斷請求是否被觸發**。

##### MIP_MTIP：
代表監管者中斷優先級寄存器中的「機器計時器中斷請求」位元（Machine Timer Interrupt Pending）。這個位元表示**機器模式下的計時器中斷請求是否被觸發**。

##### MIP_SEIP：
代表監管者中斷優先級寄存器中的「監管者外部中斷請求」位元（Supervisor External Interrupt Pending）。這個位元表示**監管者模式下的外部中斷請求是否被觸發**。

##### MIP_MEIP：
代表監管者中斷優先級寄存器中的「機器外部中斷請求」位元（Machine External Interrupt Pending）。這個位元表示**機器模式下的外部中斷請求是否被觸發**。

這些位元遮罩可以用於檢查相應的中斷請求是否被觸發，以便在程式中進行中斷處理相關的操作。**透過位元運算，可以對這些位元遮罩進行操作和判斷，以確定相應的中斷狀態**。

---

### 頁面大小設置

```c
#define PAGE_SIZE 4096 /* should be configurable */
```
`#define PAGE_SIZE 4096` 這行程式碼定義了一個名稱為 PAGE_SIZE 的巨集（宏），並將其值設置為 4096。這個巨集的目的是定義頁面的大小。

在作業系統和虛擬記憶體管理中，**頁面是內存分配的基本單位**。頁面大小通常是固定的，而且在不同的作業系統和硬體平台上可能會有所不同。在這個例子中，將頁面大小設置為 4096 位元組，即 4KB。

---

### 定義異常

這段程式碼定義了一個名為 exception_t 的列舉型別（enum）。**enum是一個用於定義一組命名常數的方式**。

```c
typedef enum {
    OK = -1,
    INSTRUCTION_ADDRESS_MISALIGNED = 0,
    INSTRUCTION_ACCESS_FAULT = 1,
    ILLEGAL_INSTRUCTION = 2,
    BREAKPOINT = 3,
    LOAD_ADDRESS_MISALIGNED = 4,
    LOAD_ACCESS_FAULT = 5,
    STORE_AMO_ADDRESS_MISALIGNED = 6,
    STORE_AMO_ACCESS_FAULT = 7,
    INSTRUCTION_PAGE_FAULT = 12,
    LOAD_PAGE_FAULT = 13,
    STORE_AMO_PAGE_FAULT = 15,
} exception_t;
```
在這個例子中，exception_t 列舉型別用於表示異常（exception）的類型

每個列舉常數都有一個名稱和與之相關聯的整數值。在這個例子中，列舉常數表示了不同類型的異常，並且**使用整數值來表示每個異常的唯一識別碼。**

其中列舉常數 OK 被指定為 -1，表示一個特殊的正常狀態。其他的列舉常數則按照它們代表的異常類型分別指定了整數值。

透過這個列舉型別，程式碼中的其他部分可以使用 exception_t 作為一個型別來聲明變數，，並將這些列舉常數賦值給這些變數以表示相應的異常狀態，**使用這些列舉常數來表示和處理不同類型的異常**。**例如，可以使用 exception_t 型別的變數來捕獲和處理異常，根據異常的類型進行相應的處理邏輯 (比如使用 switch 語句根據不同的異常類型執行相應的操作)**。

以下是對每個列舉常數的詳細解釋：

- OK：這是一個特殊的列舉常數，代表正常的狀態。在這個例子中，它被指定為 -1，表示正常狀態。
接下來是表示不同異常類型的列舉常數：

- INSTRUCTION_ADDRESS_MISALIGNED：指令地址不對齊異常。
- INSTRUCTION_ACCESS_FAULT：指令訪問錯誤異常。
- ILLEGAL_INSTRUCTION：非法指令異常。
- BREAKPOINT：斷點異常。
- LOAD_ADDRESS_MISALIGNED：讀取地址不對齊異常。
- LOAD_ACCESS_FAULT：讀取訪問錯誤異常。
- STORE_AMO_ADDRESS_MISALIGNED：存儲/原子操作地址不對齊異常。
- STORE_AMO_ACCESS_FAULT：存儲/原子操作訪問錯誤異常。
- INSTRUCTION_PAGE_FAULT：指令頁錯誤異常。
- LOAD_PAGE_FAULT：讀取頁錯誤異常。
- STORE_AMO_PAGE_FAULT：存儲/原子操作頁錯誤異常。

---

### 定義中斷

```c
typedef enum {
    NONE = -1,
    SUPERVISOR_SOFTWARE_INTERRUPT = 1,
    MACHINE_SOFTWARE_INTERRUPT = 3,
    SUPERVISOR_TIMER_INTERRUPT = 5,
    MACHINE_TIMER_INTERRUPT = 7,
    SUPERVISOR_EXTERNAL_INTERRUPT = 9,
    MACHINE_EXTERNAL_INTERRUPT = 11,
} interrupt_t;
```
以下是對每個列舉常數的詳細解釋：

- NONE：這是一個特殊的列舉常數，代表沒有中斷。在這個例子中，它被指定為 -1，用於**表示沒有中斷發生**。

接下來是表示不同中斷類型的列舉常數：

- SUPERVISOR_SOFTWARE_INTERRUPT：監管模式軟體中斷。
- MACHINE_SOFTWARE_INTERRUPT：機器模式軟體中斷。
- SUPERVISOR_TIMER_INTERRUPT：監管模式計時器中斷。
- MACHINE_TIMER_INTERRUPT：機器模式計時器中斷。
- SUPERVISOR_EXTERNAL_INTERRUPT：監管模式外部中斷。
- MACHINE_EXTERNAL_INTERRUPT：機器模式外部中斷。

---

### 致命例外判斷函式

```c
bool exception_is_fatal(const exception_t e)
{
    switch (e) {
    case INSTRUCTION_ADDRESS_MISALIGNED:
    case INSTRUCTION_ACCESS_FAULT:
    case LOAD_ACCESS_FAULT:
    case STORE_AMO_ADDRESS_MISALIGNED:
    case STORE_AMO_ACCESS_FAULT:
        return true;
    default:
        return false;
    }
}
```

這段程式碼定義了一個名為 exception_is_fatal 的函式，用於**判斷傳入的 exception_t 參數是否屬於致命例外**。

該函式使用 switch 陳述式對 exception_t 參數進行匹配，並將它與一系列致命例外的列舉常數進行比較。如果 exception_t 參數匹配到其中任何一個列舉常數，則表示**該例外是致命的，函式會返回 true**。如果 exception_t 參數不匹配任何列舉常數，則表示該例外不是致命的，函式會返回 false。

在這個特定的例子中，根據 case 條件，只有當 exception_t 參數等於以下列舉常數時，函式會返回 true：

- INSTRUCTION_ADDRESS_MISALIGNED：**指令位址不對齊**
- INSTRUCTION_ACCESS_FAULT：**指令存取錯誤**
- LOAD_ACCESS_FAULT：**讀取存取錯誤**
- STORE_AMO_ADDRESS_MISALIGNED：**存儲/原子操作位址不對齊**
- STORE_AMO_ACCESS_FAULT：**存儲/原子操作存取錯誤**

<details>
  <summary>原子操作解釋</summary>

>在計算機科學中，原子操作（Atomic Operation）是 **指不可被中斷的單個操作。它是一個不可分割的操作，要麼完全執行，要麼完全不執行，沒有中間狀態** 。原子操作 **通常用於多線程編程環境中，以確保共享資源的一致性和避免競爭條件** 。
>
>在存儲/原子操作存取錯誤（STORE_AMO_ACCESS_FAULT）的上下文中，原子操作通常指的是對共享資源進行讀取、修改或寫入的操作，並且在執行期間需要保證該操作的原子性。這意味著在進行原子操作期間，沒有其他執行緒或處理器可以同時訪問或修改相同的資源，從而避免了競爭條件和不一致的結果。
>
>存儲/原子操作存取錯誤（STORE_AMO_ACCESS_FAULT）表示在進行存儲或原子操作期間發生了存取錯誤。這可能是由於不正確的存取權限、位址不對齊或其他存取錯誤導致的。當這種錯誤發生時，系統可能會觸發例外或錯誤處理程序，以便進行必要的處理或恢復操作。
>
>總結來說，存儲/原子操作存取錯誤是指在執行存儲或原子操作期間發生了存取資源的錯誤，這些操作需要保證原子性以確保多線程環境中的資源一致性。

</details>

</br>

如果 exception_t 參數不是這些列舉常數之一，則函式會返回 false，表示該例外不是致命的。

這個函式**可以用於確定某個例外是否需要引起程式的終止** **或其他特殊處理**。

---

### RAM定義

```c
// RAM 記憶體
struct ram {
    uint8_t *data;
};
```

這段程式碼定義了一個名為 ram 的結構體，用於表示 RAM（Random Access Memory）記憶體。

該結構體包含一個指向 uint8_t 類型的指標 data。**這個指標指向 RAM 記憶體的起始位置或數據緩衝區**。uint8_t 是一個無符號8位整數類型，表示每個記憶體單元存儲的位元組數據。

通過定義 ram 結構體，可以在程式中創建 RAM 對象，並**使用 data 指標來訪問和操作 RAM 記憶體中的數據**。這可以用於模擬或處理計算機系統中的主記憶體，例如讀取或寫入指令和數據。

#### RAM創建與初始化

```c
struct ram *ram_new(const uint8_t *code, const size_t code_size)
{
    struct ram *ram = calloc(1, sizeof(struct ram));
    ram->data = calloc(RAM_SIZE, 1);
    memcpy(ram->data, code, code_size);
    return ram;
}
```

這段程式碼定義了一個名為 ram_new 的函數，用於創建並初始化一個 RAM（Random Access Memory）對象。

函數接受兩個參數：`code` 和 `code_size`。`code` 是一個指向代碼數據的指標，`code_size` 是代碼數據的大小。

在函數內部，它首先使用 calloc 函數分配了一個 ram 結構體的記憶體空間，並將其初始化為零。sizeof(struct ram) 確保分配的記憶體大小足夠容納 ram 結構體。

接下來，它再次**使用 calloc 函數分配了一個大小為 RAM_SIZE 的記憶體區域，並將其初始化為零**。這將作為 RAM 對象的數據緩衝區。

然後，**使用 memcpy 函數將 code 指向的代碼數據從 code_size 大小的內存區域複製到 RAM 對象的數據緩衝區 ram->data 中**。

**最後，函數返回創建的 RAM 對象的指標**。

總結來說，這段程式碼實現了一個函數，用於**初始化 RAM 記憶體結構，並將代碼內容複製到 RAM 中，方便後續使用和處理**。這樣**可以在模擬或處理計算機系統中的主記憶體時，使用該 RAM 對象**。

---

### RAM讀取函式

```c
exception_t ram_load(const struct ram *ram,
                     const uint64_t addr,
                     const uint64_t size,
                     uint64_t *result)
{
    uint64_t index = addr - RAM_BASE, tmp = 0;
    switch (size) {
    case 64:
        tmp |= (uint64_t)(ram->data[index + 7]) << 56;
        tmp |= (uint64_t)(ram->data[index + 6]) << 48;
        tmp |= (uint64_t)(ram->data[index + 5]) << 40;
        tmp |= (uint64_t)(ram->data[index + 4]) << 32;
    case 32:
        tmp |= (uint64_t)(ram->data[index + 3]) << 24;
        tmp |= (uint64_t)(ram->data[index + 2]) << 16;
    case 16:
        tmp |= (uint64_t)(ram->data[index + 1]) << 8;
    case 8:
        tmp |= (uint64_t)(ram->data[index + 0]) << 0;
        *result = tmp;
        return OK;
    default:
        return LOAD_ACCESS_FAULT;
    }
}
```

這段程式碼實現了一個名為 ram_load 的函數，**用於從 RAM 記憶體中讀取數據**。該函數接受四個參數：ram（指向 RAM 記憶體結構的指針）、addr（要讀取的記憶體地址）、size（要讀取的數據大小）和 result（存放讀取結果的指針）。

在函數內部，首先計算了相對於 RAM 基地址的偏移量，即 `index = addr - RAM_BASE`。然後使用 switch 語句根據 size 的大小進行不同的操作。

- 如果 size 為 64，則**依次從 ram->data 中讀取 8 個字節數據**，並將其**逐位放入 tmp 變量中**，**最終得到一個 64 位的數值**。
- 如果 size 為 32，則從 ram->data 中讀取 4 個字節數據，並將其逐位放入 tmp 變量的高 32 位中。
- 如果 size 為 16，則從 ram->data 中讀取 2 個字節數據，並將其逐位放入 tmp 變量的高 16 位中。
- 如果 size 為 8，則從 ram->data 中讀取 1 個字節數據，並將其放入 tmp 變量的低 8 位中。

**最後，將 tmp 的值賦給 result 指向的內存位置**，並**返回 OK** 表示讀取操作成功。

<details>
  <summary> tmp |= (uint64_t)(ram->data[index + 7]) << 56 程式碼解釋</summary>

>這段程式碼是將 `ram->data[index + 7]` 的值往左位移 56 個位元，然後將結果與 tmp 做位元或運算後，將結果存回 tmp。
>
>讓我們逐步解釋這個動作：
>
>`(ram->data[index + 7])`：這部分表示從 ram 的 data 陣列中取得索引為 index + 7 的元素的值。index 的計算是基於給定的 addr 值與 RAM_BASE 之間的差值。
>
>`(uint64_t)`：這部分是將 `ram->data[index + 7]` 的值轉換為 uint64_t 型別，以符合 tmp 的型別。
>
>`<< 56`：這是位元左移運算子，將 `(uint64_t)(ram->data[index + 7])` 的值向左移動 56 個位元。
>
>`tmp |= ...`：這是位元或運算子，將位元左移運算的結果與 tmp 做位元或運算，將結果存回 tmp。這樣做可以將位元左移運算的結果累積到 tmp 中。
>
>**總結來說，這行程式碼的目的是將 `ram->data[index + 7]` 的值向左位移 56 個位元，然後將結果存回 tmp**。**這樣的操作通常用於組合多個位元以形成一個大型的整數值**。在這個程式碼中，根據 size 的不同，類似的位元操作會在不同的位元位置上進行，以組合成最終的 64 位整數值 tmp。

</details>

如果 size 不屬於上述情況，則表示讀取大小不正確，函數將返回 LOAD_ACCESS_FAULT 表示讀取操作失敗。

總結來說，這段程式碼實現了一個函數，用於從 RAM 記憶體中按指定大小讀取數據，並將結果存儲在指定的內存位置中。

---

### RAM寫入函式


```c
exception_t ram_store(struct ram *ram,
                      const uint64_t addr,
                      const uint64_t size,
                      const uint64_t value)
{
    uint64_t index = addr - RAM_BASE;
    switch (size) {
    case 64:
        ram->data[index + 7] = (value >> 56) & 0xff;
        ram->data[index + 6] = (value >> 48) & 0xff;
        ram->data[index + 5] = (value >> 40) & 0xff;
        ram->data[index + 4] = (value >> 32) & 0xff;
    case 32:
        ram->data[index + 3] = (value >> 24) & 0xff;
        ram->data[index + 2] = (value >> 16) & 0xff;
    case 16:
        ram->data[index + 1] = (value >> 8) & 0xff;
    case 8:
        ram->data[index + 0] = (value >> 0) & 0xff;
        return OK;
    default:
        return STORE_AMO_ACCESS_FAULT;
    }
}
```

這段程式碼是用於將數值 value 存儲到 RAM 的特定地址 addr 中。**與之前提到的 ram_load 函式類似**，這裡也根據 size 參數的大小將 value 拆分為不同的位元組，然後將它們存儲到 RAM 中的相應位元組位置。

根據 size 的不同情況，進行如下操作：

- 如果 size 是 64，則需要將 value 的 64 位拆分成 8 個位元組，並將每個位元組**存儲到 RAM 的相應位元組位置上**。

- 如果 size 是 32，則需要將 value 的 32 位拆分成 4 個位元組，並將每個位元組存儲到 RAM 的相應位元組位置上。

- 如果 size 是 16，則需要將 value 的 16 位拆分成 2 個位元組，並將每個位元組存儲到 RAM 的相應位元組位置上。

- 如果 size 是 8，則只需要將 value 的最低 8 位存儲到 RAM 中的相應位元組位置上。

通過根據 size 的值進行不同的操作，我們確保將 value 正確地存儲到 RAM 中的相應位元組位置上。這樣可以將給定大小的數據存儲到 RAM 中，以便後續的讀取操作可以獲得正確的值。

---

### 函式 : 顯示錯誤訊息並終止程式執行

```c
static void fatal(const char *msg)
{
    fprintf(stderr, "ERROR: Failed to %s.\n", msg);
    exit(1);
}
```
這段程式碼定義了一個名為 fatal 的靜態函式，其功能是在發生嚴重錯誤時顯示錯誤訊息並終止程式執行。

具體來說，這個函式接受一個字串 msg 作為參數，該字串描述了發生錯誤的操作或原因。它使用 fprintf 函式將錯誤訊息輸出到標準錯誤輸出流（stderr）。格式化字串 `"ERROR: Failed to %s.\n"` 中的` %s` 會被 msg 取代，以產生最終的錯誤訊息。

最後，它呼叫 `exit` 函式結束程式的執行，並以參數值 `1` 作為程式的結束碼。這表示程式在發生致命錯誤後會以非正常的方式終止。

總結來說，**這段程式碼用於顯示錯誤訊息並終止程式的執行，以應對嚴重錯誤情況**。

---

### 局部中斷器定義

Core Local Interruptor（CLINT）局部中斷器，是一個位於 RISC-V 處理器中的硬體模組，用於**提供時間相關的功能**，**例如計時器和時脈中斷**。

```c
// Core Local Interruptor (CLINT) 局部中斷 (內部)
struct clint {
    uint64_t mtime, mtimecmp; // 時間控制暫存器
};
```

在這個結構中，mtime 和 mtimecmp 是兩個**時間控制暫存器**，用於實現計時器和比較器的功能。

- mtime 是**實際的計時器暫存器**，用於**保存系統運行的時間值**。它會以某種頻率進行自增，可以**用來計算時間間隔**或**追蹤系統運行的時間**。

- mtimecmp 是**用於比較的暫存器**，可以設定一個時間值，**當 mtime 的值與 mtimecmp 的值相等時，就會觸發時脈中斷**。

這個結構定義了 CLINT 的狀態，並通過這兩個成員讓程式能夠存取和控制 CLINT 的時間相關功能。**程式可以使用這些暫存器進行時間的追蹤、中斷的觸發以及相關的操作**。

#### 局部中斷器創建與初始化

```c
struct clint *clint_new()
{
    return calloc(1, sizeof(struct clint));
}
```
這個函式使用 calloc 函式來分配記憶體以存儲 clint 結構的內容。calloc 函式會將分配的記憶體初始化為零，確保所有成員的初始值為預設值。通過將 sizeof(struct clint) 作為分配大小的參數，確保分配的記憶體大小足夠容納 clint 結構。

函式執行完畢後，會返回指向新建立的 clint 結構的指標，這樣程式**就可以使用該指標來存取和操作相關的 CLINT 功能**。

每個 CLINT 結構都包含了 mtime 和 mtimecmp 兩個時間控制暫存器，用於處理時間相關的中斷和計時功能。透過建立新的 clint 結構，**程式能夠對 CLINT 進行初始化、設置計時器等操作，以確保正確的時間控制和中斷處理機制的運作**。

---

### 函式: 讀取局部中斷器

```c
inline exception_t clint_load(const struct clint *clint,
                              const uint64_t addr,
                              const uint64_t size,
                              uint64_t *result)
{
    if (size != 64)
        return LOAD_ACCESS_FAULT;

    switch (addr) {
    case CLINT_MTIMECMP:
        *result = clint->mtimecmp;
        break;
    case CLINT_MTIME:
        *result = clint->mtime;
        break;
    default:
        *result = 0;
    M}
    return OK;
}

```

這段程式碼定義了一個內聯函式 clint_load，**透過 clint_load 函式讀取 CLINT 結構中的 mtime 或 mtimecmp 值**，**可以在程式中獲取系統運行時間信息或設置定時中斷閾值，從而進行相應的操作和控制**。
它**接受一個指向 clint 結構的指針**，**要讀取的地址 addr**，要讀取的數據大小 size，以及一個指向 result 的指針，用於存儲讀取的結果。

函式首先檢查指定的數據大小是否為 64 位，如果不是，則返回 LOAD_ACCESS_FAULT，表示讀取錯誤。

接下來，**根據指定的地址 addr 進行 switch-case 分支判斷**，以**確定要讀取的數據是 clint 結構中的哪個成員**。如果地址是 CLINT_MTIMECMP，則將 clint->mtimecmp 的值存儲到 result 中；如果地址是 CLINT_MTIME，則將 clint->mtime 的值存儲到 result 中；如果地址不是以上兩者，則將 result 設置為 0。

最後，函式返回 OK，表示讀取操作成功。

---

### 函式: 寫入局部中斷器

```c
inline exception_t clint_store(struct clint *clint,
                               const uint64_t addr,
                               const uint64_t size,
                               const uint64_t value)
{
    if (size != 64)
        return STORE_AMO_ACCESS_FAULT;

    switch (addr) {
    case CLINT_MTIMECMP:
        clint->mtimecmp = value;
        break;
    case CLINT_MTIME:
        clint->mtime = value;
        break;
    }
    return OK;
}
```

這段程式碼是 clint_store 函式，它用於**將指定的數值存儲到 CLINT (Core Local Interruptor) 結構中的特定暫存器中**。

根據程式碼，它會先檢查存儲的大小是否為 64 位。如果不是，則會返回 STORE_AMO_ACCESS_FAULT，表示存儲操作失敗或不支持。若大小符合要求，則會根據給定的地址 addr，將數值 value 存儲到相應的暫存器中。

具體而言，根據 addr 的值，它會進行以下操作：

- 如果 addr 等於 CLINT_MTIMECMP，則將 value 存儲到 CLINT 結構的 mtimecmp 暫存器中。
- 如果 addr 等於 CLINT_MTIME，則將 value 存儲到 CLINT 結構的 mtime 暫存器中。

這個**函式的目的是更新 CLINT 結構中的特定暫存器的值，以實現對系統計時器或定時中斷閾值的設置**。具體的使用方式和目的取決於程式中對存儲操作的需求和後續的系統邏輯。

---

### 定義PLIC(管理全局中斷的外部中斷控制器)

```c
// Platform-Level Interrupt Controller (PLIC) 全局中斷 (外部)
struct plic {
    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};
```

這段程式碼定義了 PLIC (Platform-Level Interrupt Controller) 結構，用於管理全局中斷的外部中斷控制器。

該結構包含以下成員：

- pending：表示**待處理的中斷位元**，**用於追蹤尚未被處理的中斷**。
- senable：表示使能中斷位元，**用於控制哪些中斷可以被觸發並被處理**。
- spriority：表示中斷優先級位元，用**於設置每個中斷的優先級**。
- sclaim：表示中斷索取位元，用於**索取處理中的中斷**。

<details>
  <summary> 索取處理中的中斷，解釋</summary>

>「索取處理中的中斷」（Claiming Interrupts）是一種機制，用於多處理器系統中的中斷處理。當多個處理器核心同時遇到可處理的中斷時，為了避免多個核心同時處理同一個中斷，系統中斷控制器（如PLIC）會提供一種機制，允許處理器核心索取（claim）要處理的中斷。
>
>當中斷事件發生時，中斷控制器會標記該中斷為「處理中」狀態。然後，**處理器核心可以向中斷控制器發出索取（claim）該中斷的請求**。**中斷控制器會選擇一個處於「處理中」狀態的中斷，並將其標記為「已索取」狀態，表示該處理器核心將處理該中斷。** 這樣可以 **確保系統中的不同處理器核心能夠分配到不同的中斷，避免重複處理相同的中斷**。
>
>這種索取處理中的中斷機制有助於確保中斷處理的協調性和效率，特別在多處理器系統中非常重要，以確保中斷處理的互斥性和順序性。

</details>
</br>

這些成員將用於**實現全局中斷控制器的基本功能**，包括**檢查待處理的中斷**、**使能特定中斷**、**設置中斷優先級**以及**索取處理中的中斷**等操作。具體的使用方式和目的將取決於系統的中斷管理需求和外部設備的中斷觸發。

#### PLIC(管理全局中斷的外部中斷控制器)創建與初始化

```c
struct plic *plic_new()
{
    return calloc(1, sizeof(struct plic));
}
```
這段程式碼 plic_new() 是用於創建一個新的 plic（全局中斷控制器）結構。它使用 calloc() 函數動態分配了一塊記憶體，大小為 sizeof(struct plic)，並將該記憶體初始化為零。然後，它**將該結構的指標返回。**

簡單來說，這段程式碼創建了一個新的 plic 結構並將其初始化，以便在後續的程式中使用。

---

### 函式 : PLIC讀與寫

PLIC(管理全局中斷的外部中斷控制器)

底下兩段程式碼定義了兩個函數：`plic_load` 和 `plic_store`，用於讀取和寫入 `plic`（全局中斷控制器）結構中的相應成員。


```c
exception_t plic_load(const struct plic *plic,
                      const uint64_t addr,
                      const uint64_t size,
                      uint64_t *result)
{
    if (size != 32)
        return LOAD_ACCESS_FAULT;

    switch (addr) {
    case PLIC_PENDING:
        *result = plic->pending;
        break;
    case PLIC_SENABLE:
        *result = plic->senable;
        break;
    case PLIC_SPRIORITY:
        *result = plic->spriority;
        break;
    case PLIC_SCLAIM:
        *result = plic->sclaim;
        break;
    default:
        *result = 0;
    }
    return OK;
}
```

`plic_load` 函數根據給定的地址（`addr`）和大小（`size`），從 `plic` 結構中讀取對應的值並存儲在 `result` 指向的變數中。如果指定的大小不為 32，則返回 `LOAD_ACCESS_FAULT` 异常。**根據給定的地址，它將讀取 `plic` 結構中的 `pending`、`senable`、`spriority` 或 `sclaim` 成員的值。**

```c
exception_t plic_store(struct plic *plic,
                       const uint64_t addr,
                       const uint64_t size,
                       const uint64_t value)
{
    if (size != 32)
        return STORE_AMO_ACCESS_FAULT;

    switch (addr) {
    case PLIC_PENDING:
        plic->pending = value;
        break;
    case PLIC_SENABLE:
        plic->senable = value;
        break;
    case PLIC_SPRIORITY:
        plic->spriority = value;
        break;
    case PLIC_SCLAIM:
        plic->sclaim = value;
        break;
    }
    return OK;
}
```
`plic_store` 函數根據給定的地址（`addr`）和大小（`size`），將給定的值（`value`）寫入到 `plic` 結構的相應成員中。如果指定的大小不為 32，則返回 `STORE_AMO_ACCESS_FAULT` 异常。根據給定的地址，它將寫入 `plic` 結構中的 `pending`、`senable`、`spriority` 或 `sclaim` 成員。

總之，這兩段程式碼提供了對 `plic` 結構進行讀取和寫入操作的功能，以便進行全局中斷的控制和管理。

---

### 通用非同步收發傳輸器定義

UART（通用非同步收發傳輸器）是一種通信協議，**用於在電腦和外部設備（如終端機）之間傳輸數據**。

```c
// UART (透過 UART 傳送/接收 終端機 的訊息)
struct uart {
    uint8_t data[UART_SIZE];
    bool interrupting;

    pthread_t tid;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};
```
該結構包含以下成員：

- data：用於**存儲 UART 接收和傳輸的數據**。這是一個長度為 UART_SIZE 的數組。
- interrupting：**表示是否有中斷正在發生**，用於同步和控制中斷的處理。
- tid：用於**存儲 UART 线程的 ID（識別符）**。
- lock：**用於線程同步的互斥鎖**，用於保護對 UART 結構的临界區。
- cond：**用於線程同步的條件變數，用於在特定條件下阻塞和喚醒線程**。

這個結構**提供了在 UART 中傳送和接收數據的線程化支持**。它包含**數據緩衝區**、**同步機制**和**相關線程的信息**，以**實現可靠的通信和對外部設備的控制**。

---

### 函式: UART輸入

```c
static void *uart_thread_func(void *priv)
{
    struct uart *uart = (struct uart *) priv;
    while (1) {
        char c;             //用於存儲讀取的字符
        if (read(STDIN_FILENO, &c, 1) <= 0)  /* an error or EOF */
            continue;       // 如果讀取錯誤或遇到文件結束（EOF），則繼續下一次迴圈。
        pthread_mutex_lock(&uart->lock);
        while ((uart->data[UART_LSR - UART_BASE] & UART_LSR_RX) == 1)
            pthread_cond_wait(&uart->cond, &uart->lock);

        uart->data[0] = c;
        uart->interrupting = true;
        uart->data[UART_LSR - UART_BASE] |= UART_LSR_RX;
        pthread_mutex_unlock(&uart->lock);  //解鎖 UART 結構的互斥鎖，釋放其他線程的訪問權限。
    }

    /* should not reach here */
    return NULL;
}
```

這段程式碼定義了 UART 的線程函數 `uart_thread_func`，該函數在一個無限迴圈中運行，負責從標準輸入（`STDIN_FILENO`）讀取字符並將其存儲到 UART 的數據緩衝區中。

函數的主要步驟如下：
1. 定義一個字符變量 `c` 用於存儲讀取的字符。
2. 使用 `read` 函數從標準輸入讀取一個字符，並將其存儲到 `c` 中。如果讀取錯誤或遇到文件結束（EOF），則繼續下一次迴圈。
3. **使用 `pthread_mutex_lock` 函數鎖定 UART 結構的互斥鎖，以確保線程安全**。
4. 使用條件變數 `pthread_cond_wait` 在接收緩衝區滿時進行等待，直到有空間可用。
5. 將讀取的字符 `c` 存儲到 UART 的數據緩衝區中（通常是 `uart->data[0]`）。
6. 將 `uart->interrupting` 設置為 `true`，表示中斷正在發生。
7. 將 UART 狀態寄存器 `uart->data[UART_LSR - UART_BASE]` 的接收就緒標誌位（`UART_LSR_RX`）設置為 1，表示數據已接收。
8. 使用 `pthread_mutex_unlock` 解鎖 UART 結構的互斥鎖，釋放其他線程的訪問權限。
9. 回到第1步，繼續循環等待下一個字符的讀取。

該線程函數是在單獨的線程中運行，並且負責從標準輸入讀取字符並將其寫入 UART 的數據緩衝區，以模擬從終端機接收數據的操作。

### 建立並初始化 UART 結構，啟動 UART 輸入執行緒

```c
struct uart *uart_new()
{
    struct uart *uart = calloc(1, sizeof(struct uart));
    uart->data[UART_LSR - UART_BASE] |= UART_LSR_TX;

    //初始化互斥鎖 (`uart->lock`) 和條件變數 (`uart->cond`)，用於執行緒同步。
    pthread_mutex_init(&uart->lock, NULL);
    pthread_cond_init(&uart->cond, NULL);

    pthread_create(&uart->tid, NULL, uart_thread_func, (void *) uart);
    return uart;
}
```

這段程式碼定義了一個名為 `uart_new` 的函式，其目的是建立並初始化 UART 結構。該函式執行以下操作：

1. 分配一塊記憶體，以容納 `struct uart` 的結構。
2. 將 `uart` 結構的內容初始化為零。
3. 將 UART 狀態寄存器 (`uart->data[UART_LSR - UART_BASE]`) 的 TX 標誌位置為 1，表示 UART 可以進行傳輸。
4. 初始化互斥鎖 (`uart->lock`) 和條件變數 (`uart->cond`)，用於執行緒同步。
5. **創建一個新的執行緒 (`uart->tid`)，並將其設置為執行 `uart_thread_func` 函式，並將 `uart` 結構作為參數傳遞給該函式。**
6. 返回初始化後的 UART 結構。

---

### 函式: UART 結構的讀取和寫入

```c
exception_t uart_load(struct uart *uart,
                      const uint64_t addr,
                      const uint64_t size,
                      uint64_t *result)
{
    if (size != 8)
        return LOAD_ACCESS_FAULT;

    pthread_mutex_lock(&uart->lock);
    switch (addr) {
    case UART_RHR:
        pthread_cond_broadcast(&uart->cond);
        uart->data[UART_LSR - UART_BASE] &= ~UART_LSR_RX;
    default:
        *result = uart->data[addr - UART_BASE];
    }
    pthread_mutex_unlock(&uart->lock);
    return OK;
}

exception_t uart_store(struct uart *uart,
                       const uint64_t addr,
                       const uint64_t size,
                       const uint64_t value)
{
    if (size != 8)
        return STORE_AMO_ACCESS_FAULT;

    pthread_mutex_lock(&uart->lock);
    switch (addr) {
    case UART_THR:
        putchar(value & 0xff);
        fflush(stdout);
        break;
    default:
        uart->data[addr - UART_BASE] = value & 0xff;
    }
    pthread_mutex_unlock(&uart->lock);
    return OK;
}
```
這段程式碼定義了兩個函式：`uart_load` 和 `uart_store`，用於處理對 UART 結構的讀取和寫入操作。

`uart_load` 函式的功能如下：
- 首先，檢查所需讀取的資料大小是否為 8 位元組，如果不是則返回 `LOAD_ACCESS_FAULT` 表示讀取存取錯誤。
- 進入互斥鎖保護的區塊，以防止多個執行緒同時訪問 UART 結構。
- 根據給定的位址 `addr` 進行判斷：
  - 若 `addr` 等於 `UART_RHR`，表示要讀取 UART 接收暫存器，此時觸發條件變數的廣播操作以喚醒可能正在等待讀取的執行緒，並將 UART 狀態寄存器中的接收位元清除。
  - 其他情況下，將給定位址 `addr` 對應的 UART 資料寄存器的值存入 `result` 中。
- 解鎖互斥鎖，並返回 `OK` 表示操作成功。

`uart_store` 函式的功能如下：
- 首先，檢查所需寫入的資料大小是否為 8 位元組，如果不是則返回 `STORE_AMO_ACCESS_FAULT` 表示寫入存取錯誤。
- 進入互斥鎖保護的區塊，以防止多個執行緒同時訪問 UART 結構。
- 根據給定的位址 `addr` 進行判斷：
  - 若 `addr` 等於 `UART_THR`，表示要寫入 UART 傳輸暫存器，此時將 `value` 的低 8 位元寫入標準輸出並刷新緩衝區，以進行輸出。
  - 其他情況下，將 `value` 的低 8 位元存入 UART 資料寄存器對應的位址 `addr` 中。
- 解鎖互斥鎖，並返回 `OK` 表示操作成功。


---