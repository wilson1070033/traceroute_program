# Enhanced Visual Traceroute

一個功能增強的命令列 traceroute 工具,整合地理位置、ISP 資訊與進階網路指標分析。

## 專案簡介

本專案是傳統 traceroute 工具的加強版實作,透過 ICMP Echo Request 封包追蹤網路路由路徑,並提供豐富的視覺化輸出與詳細的網路統計資訊。

### 核心功能

- **視覺化路由追蹤**: 使用 ANSI 色彩與圖形符號呈現路由路徑
- **地理位置定位**: 顯示每個節點的國家、城市、經緯度座標
- **ISP/ASN 資訊**: 查詢網際網路服務供應商與自治系統編號
- **延遲分析**: 計算平均值、最小值、最大值、標準差與抖動(Jitter)
- **封包遺失率**: 統計每個跳點的封包遺失百分比
- **距離計算**: 使用 Haversine 公式計算相鄰節點間的地理距離
- **路由摘要**: 提供端到端的統計分析報告

## 技術架構

### 系統需求

- **作業系統**: Linux (需支援 Raw Socket)
- **編譯器**: g++ 支援 C++11 或更新版本
- **權限**: Root 權限(使用 Raw Socket 發送 ICMP 封包)

### 外部依賴函式庫

```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev

# Fedora/RHEL
sudo dnf install libcurl-devel json-devel

# Arch Linux
sudo pacman -S curl nlohmann-json
```

**必要套件**:
- `libcurl`: HTTP 請求處理(查詢地理位置 API)
- `nlohmann/json`: JSON 資料解析
- 標準 Linux Socket API (`sys/socket.h`, `netinet/ip_icmp.h`)

### API 服務

使用 [ip-api.com](http://ip-api.com) 免費服務查詢 IP 地址資訊:
- 地理位置資訊(國家、城市、經緯度)
- ISP 與 ASN 資料
- **限制**: 免費方案每分鐘 45 次請求

## 編譯與安裝

### 編譯指令

```bash
g++ -o traceroute main.cpp traceroute.cpp network.cpp display.cpp utils.cpp \
    -lcurl -std=c++11 -O2
```

### Makefile 範例

```makefile
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall
LDFLAGS = -lcurl

TARGET = traceroute
SOURCES = main.cpp traceroute.cpp network.cpp display.cpp utils.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
```

編譯:
```bash
make
```

## 使用方式

### 基本用法

```bash
sudo ./traceroute <目標主機名稱或 IP>
```

### 範例

```bash
# 追蹤到 Google DNS 的路由
sudo ./traceroute 8.8.8.8

# 追蹤到特定網域
sudo ./traceroute www.google.com

# 追蹤到台灣學術網路
sudo ./traceroute www.twaren.net
```

### 輸出範例

程式會顯示以下資訊:

1. **標頭資訊**: 目標主機、探測參數
2. **逐跳詳細資料**:
   - 跳點編號與主機名稱
   - IP 位址
   - 地理位置(國家、城市、經緯度)
   - 與前一跳的距離(公里)
   - ISP 與 ASN 資訊
   - 延遲統計(平均、最小、最大、標準差、抖動)
   - 視覺化延遲長條圖
   - 封包遺失率
3. **路由摘要統計**:
   - 總跳點數與成功率
   - 端到端總延遲
   - 平均延遲與抖動
   - 總地理距離與訊號傳播速度估算

## 專案結構

```
traceroute_program/
├── common_types.h      # 資料結構定義與常數設定
├── display.h/cpp       # 視覺化輸出與格式化函式
├── network.h/cpp       # 網路操作(API 查詢、ICMP 封包)
├── traceroute.h/cpp    # 核心 traceroute 邏輯
├── utils.h/cpp         # 工具函式(數學計算、checksum)
└── main.cpp            # 程式進入點
```

### 模組說明

**common_types.h**
- 定義 `GeoInfo`、`NetworkInfo`、`EnhancedHopInfo` 資料結構
- 設定全域常數(最大跳數、逾時時間、封包大小)
- ANSI 色彩代碼定義

**network.cpp/h**
- `query_geo_info()`: 查詢地理位置資訊
- `query_network_info()`: 查詢 ISP/ASN 資料
- `create_icmp_packet()`: 建立 ICMP Echo Request 封包
- `resolve_hostname()`: DNS 名稱解析

**traceroute.cpp/h**
- `enhanced_traceroute()`: 主要追蹤邏輯
  - 建立 Raw Socket
  - 設定 TTL 值逐步遞增
  - 發送 ICMP 封包並接收回應
  - 計算 RTT 與統計資料
  - 呼叫 API 查詢額外資訊

**display.cpp/h**
- `print_header()`: 輸出程式標頭
- `print_hop_entry()`: 顯示單一跳點資訊
- `print_summary()`: 輸出統計摘要
- `draw_latency_bar()`: 繪製延遲視覺化長條圖
- `get_latency_color()`: 根據延遲值選擇顏色

**utils.cpp/h**
- `calculate_distance()`: Haversine 公式計算地球表面兩點距離
- `calculate_std_dev()`: 標準差計算
- `calculate_jitter()`: 抖動計算
- `calculate_checksum()`: ICMP checksum 計算
- `WriteCallback()`: libcurl 資料接收回呼函式

## 設定參數

可在 `common_types.h` 中調整以下參數:

```cpp
#define MAX_HOPS 30              // 最大跳數
#define PACKET_SIZE 64           // ICMP 封包大小(位元組)
#define TIMEOUT_SEC 2            // 接收逾時時間(秒)
#define PROBES_PER_HOP 1         // 每個跳點的探測次數
#define BAR_REFERENCE_RTT 100.0  // 延遲長條圖參考值(毫秒)
```

## 技術原理

### ICMP Traceroute 運作機制

1. **TTL (Time To Live) 遞增策略**:
   - 從 TTL=1 開始發送 ICMP Echo Request
   - 每個路由器收到封包後 TTL 減 1
   - TTL 歸零時路由器回傳 ICMP Time Exceeded
   - 透過回應來源辨識該跳點的 IP 位址

2. **RTT 測量**:
   - 記錄發送時間戳 (`gettimeofday`)
   - 接收回應後計算時間差
   - 公式: RTT = (recv_time - send_time)

3. **統計計算**:
   - **標準差**: 衡量延遲變異程度
   - **抖動**: 連續 RTT 測量值的絕對差平均
   - **封包遺失率**: (失敗次數 / 總探測次數) × 100%

## 權限與安全性

### 為何需要 Root 權限

- **Raw Socket**: 發送自訂 ICMP 封包需要 `CAP_NET_RAW` 能力
- **系統限制**: Linux 核心預設僅允許特權程序建立 Raw Socket

### 安全執行方式

1. **使用 sudo** (建議):
   ```bash
   sudo ./traceroute target.com
   ```

2. **設定 setuid** (不建議,有安全風險):
   ```bash
   sudo chown root:root traceroute
   sudo chmod u+s traceroute
   ```

3. **賦予 capabilities** (較安全):
   ```bash
   sudo setcap cap_net_raw+ep ./traceroute
   ./traceroute target.com
   ```

## 限制與注意事項

1. **API 速率限制**: ip-api.com 免費方案每分鐘 45 次請求,追蹤長路徑時可能超出限制
2. **ICMP 封鎖**: 部分網路設備或防火牆會過濾 ICMP 封包,導致顯示超時
3. **非對稱路由**: 回程路徑可能與去程不同,本工具僅顯示去程路由
4. **地理位置準確度**: IP 地理定位資料庫可能不完全準確,特別是企業網路或 CDN 節點
5. **IPv4 限制**: 目前僅支援 IPv4,未實作 IPv6

## 故障排除

### 編譯錯誤

**問題**: `fatal error: curl/curl.h: No such file or directory`

**解決方法**:
```bash
sudo apt-get install libcurl4-openssl-dev
```

**問題**: `fatal error: nlohmann/json.hpp: No such file or directory`

**解決方法**:
```bash
sudo apt-get install nlohmann-json3-dev
```

### 執行錯誤

**問題**: `Error: Root privileges required`

**解決方法**: 使用 `sudo` 執行程式

**問題**: 全部顯示 `* * * Request Timeout * * *`

**可能原因**:
- 目標主機防火牆封鎖 ICMP
- 網路連線問題
- 路由器設定過濾 ICMP

**問題**: API 查詢失敗(無地理位置資訊)

**可能原因**:
- 超出 API 速率限制
- 網路無法連接 ip-api.com
- IP 位址為私有位址(10.x.x.x, 192.168.x.x)

## 授權條款

本專案為教育用途範例程式。使用時請遵守:
- 目標網路的使用政策
- ip-api.com 服務條款
- 當地網路監測相關法規

## 參考資料

- [RFC 792 - Internet Control Message Protocol](https://tools.ietf.org/html/rfc792)
- [RFC 1071 - Computing the Internet Checksum](https://tools.ietf.org/html/rfc1071)
- [ip-api.com API Documentation](https://ip-api.com/docs)
- [Haversine Formula](https://en.wikipedia.org/wiki/Haversine_formula)
