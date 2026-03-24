# Z80ATmega128 バグ修正計画書

## 概要

Z80ATmega128 システムのコードレビューで発見されたバグの修正計画を策定する。本報告書は以下を含む。

1. バグ一覧と各バグの詳細
2. バグ間の依存関係
3. 修正の優先順位
4. 修正ステップと各ステップの詳細（修正方法を含む）

### 対象ソースコード

- AVR 側: `avr/src/` 以下
- Z80 BIOS: `z80/cpm22/bios/bios.asm`

### 運用条件

- シリアル通信速度: 9600 bps または 19200 bps

---

## 1. バグ一覧

| ID      | 重大度     | 概要 |
|---------|-----------|------|
| BUG #1  | CRITICAL  | `z80_int_vector` のレースコンディション |
| BUG #2  | CRITICAL  | `cli()` 後の `ExtMem_attach()` によるデッドロック |
| BUG #4  | MODERATE  | `x_enqueue()`/`x_dequeue()` の ISR 内 `sei()` 問題 |
| BUG #5  | LOW       | `Transmit_TX1_Buf()` に `z80_int_num_tx1 < 128` ガードがない |
| BUG #6  | CRITICAL  | `LOAD_CCP_BDOS` にリジェクション・リトライがない |
| BUG #7  | CRITICAL  | `SELDSK` が AVR 側の進行中ディスク操作を強制リセット |
| BUG #8  | IMPORTANT | Timer2 ISR (`ISR_NOBLOCK`) の再入リスク |
| BUG #9  | IMPORTANT | DEBUGGER の HALT が DI 状態で実行される |
| BUG #10 | MODERATE  | `int_level_write`/`int_level_read` に `volatile` がない |
| BUG #11 | MODERATE  | `IN_0E_DSK_WriteStatus()`/`IN_13_DSK_ReadStatus()` の `sei()` 問題 |
| BUG #12 | MODERATE  | `Enqueue_RX1_Buf()` バッファ満杯時のデータロストと偽割り込み |
| BUG #13 | MODERATE  | `ConsoleBuffer` の `count`/`head`/`tail` に `volatile` がない |
| BUG #14 | LOW       | `USART1_Transmit()` の Timer0 ISR 内ビジーウェイト（9600/19200bps） |
| BUG #15 | LOW       | CONOUT フロー制御の閾値ギャップ |

> BUG #3 は欠番。

---

## 2. 各バグの詳細

### BUG #1 [CRITICAL]: `z80_int_vector` のレースコンディション

**ファイル**: `avr/src/z80io.c` (L138-142), `avr/src/isr.c` (L64-80)

**問題のコード**:

```c
// z80io.c L138
void Z80_EXTINT_low(uint8_t vector)
{
    z80_int_vector = vector;
    CLR_BIT(PORTD, PORTD4);     // /INT = Low
}
```

**原因**: `Z80_EXTINT_low()` は以下の 4 つの ISR コンテキストから呼ばれ、単一のグローバル変数 `z80_int_vector` を排他制御なしに書き換える。

| 呼び出し元 | ISR コンテキスト | ベクタ値 |
|---|---|---|
| `em_disk_read()` 完了/reject | Timer2 (`ISR_NOBLOCK`) | 0x00 |
| `em_disk_write()` 完了/reject | Timer2 (`ISR_NOBLOCK`) | 0x02 |
| `Transmit_TX1_Buf()` | Timer0 (1ms 周期) | 0x04 |
| `Enqueue_RX1_Buf()` | USART1_RX | 0x06 |

Timer2 ISR は `ISR_NOBLOCK` のため実行中に割り込みが再許可される。`Z80_EXTINT_low()` が `/INT=Low` にした後、Z80 が INTA サイクルを開始するまでの約 2〜8µs の間に Timer0 または USART1_RX が割り込むと、`z80_int_vector` が上書きされる。`/INT` は既に Low のため 2 回目の `CLR_BIT` では Z80 側に変化が通知されず、上書き前のベクタに対応する割り込みが消失する。

**競合シナリオの詳細**:

1. Timer2 ISR 内で `em_disk_read()` が完了 → `Z80_EXTINT_low(0)` 呼び出し
2. `z80_int_vector = 0`、`/INT = LOW`
3. Z80 が `/INT` を検知、割り込みアクノレッジサイクル開始
4. **この瞬間** Timer0 ISR がプリエンプト（Timer2 は `ISR_NOBLOCK` のため可能）
5. `Transmit_TX1_Buf()` がバッファ状態変化を検知 → `Z80_EXTINT_low(4)` 呼び出し
6. **`z80_int_vector = 4` に上書き!** `/INT` は既に LOW なので変化なし
7. Timer0 ISR 復帰、AVR `ISR(INT4_vect)` が発火
8. INT4 は上書きされた `z80_int_vector = 4`（CONOUT ベクタ）を Z80 に提供
9. Z80 は **ISR_02（CONOUT handler）** に飛ぶ — 本来は ISR_00（DISK READ 完了）に飛ぶべき
10. `IS_READ_DONE` は**永久にセットされない**

**衝突確率の推定**: `Z80_EXTINT_low()` 呼び出しから INT4 発火までの時間窓は約 3〜10µs。Timer0 は 1ms 毎に発火するため、各ディスク操作での衝突確率は約 0.3〜1%。Zork I のような長時間ゲームプレイ中の多数のディスク I/O では、1 時間以内に少なくとも 1 回の衝突が発生する確率は極めて高い。

**影響**: ディスク I/O 完了割り込みが消失した場合、Z80 が以下のポーリングループに永久滞留する：

```asm
WAIT_READ_COMPLETE:
        LD A, (IS_READ_DONE)
        OR A
        JR Z, WAIT_READ_COMPLETE    ; 無限ループ
```

---

### BUG #2 [CRITICAL]: `cli()` 後の `ExtMem_attach()` によるデッドロック

**ファイル**: `avr/src/emuldev/em_diskio.c` (L339-343, L376-380, L425-429, L564-568, L575-579)

**問題のコード** (em_disk_read の例):

```c
// em_diskio.c L564
cli();
ExtMem_attach();
memcpy(dst, tmpbuf, sizeof(tmpbuf));
ExtMem_detach();
sei();
```

`ExtMem_attach()` → `Z80_BUSREQ(1)` → `/BUSACK` を無限待ちする。

**`Z80_BUSREQ()` の実装** (`z80io.c`):

```c
void Z80_BUSREQ(int st)
{
    if (st) {
        CLR_BIT(PORTD, PORTD6);       // /BUSREQ = Low
        while (PIND & _BV(PORTD7)) {  // /BUSACK を待つ（無限ループ）
        }
    }
}
```

**デッドロックシナリオの詳細**:

1. Timer2 ISR（`ISR_NOBLOCK`）実行中
2. `cli()` で全割り込み無効化
3. **この瞬間** Z80 が IN/OUT 命令実行中 → ハードウェア WAIT 回路が `/WAIT = LOW` に
4. Z80 は WAIT 状態で停止（マシンサイクル未完了）
5. AVR が `Z80_BUSREQ(1)` → `/BUSREQ = LOW`
6. Z80 は **WAIT が解除されるまで** `/BUSACK` を応答不可
7. WAIT の解除には INT0/INT1 ハンドラの実行が必要 → **`cli()` で不可能**
8. **デッドロック**: AVR は `/BUSACK` を永久に待ち、Z80 は WAIT 状態で永久停止

**影響**: AVR と Z80 の相互デッドロック。

---

### BUG #4 [MODERATE]: `x_enqueue()`/`x_dequeue()` の ISR 内 `sei()` 問題

**ファイル**: `avr/src/xconsoleio.c` (L149-153, L170-174)

**問題のコード**:

```c
// xconsoleio.c L149
cli();
cb->buffer[cb->tail] = data;
cb->tail = (cb->tail + 1) % cb->size;
cb->count++;
sei();          // ← 呼び出し元の割り込み状態を無視
```

ISR 内から呼ばれた場合、ISR 完了前に `sei()` が割り込みを再有効化する。これにより Timer0/Timer2/USART1_RX 間の予期しないプリエンプションが発生し、BUG #1 のレース窓が広がる。

**影響**: BUG #1 のレース発生確率の増大。

---

### BUG #5 [LOW]: `Transmit_TX1_Buf()` に `z80_int_num_tx1 < 128` ガードがない

**ファイル**: `avr/src/emuldev/em_consoleio.c` (L52-64)

**問題のコード**:

```c
// em_consoleio.c L58
if (cb_tx1.count == 0 ||
    cb_tx1.count == cb_tx1.size / 4 ||
    cb_tx1.count == cb_tx1.size / 2) {
    Z80_EXTINT_low(z80_int_num_tx1 << 1);   // ← ガードなし
}
```

`Enqueue_RX1_Buf()` には `z80_int_num_rx1 < 128` のガードがあるが、`Transmit_TX1_Buf()` にはない。BOOT で `z80_int_num_tx1 = 2` に設定されるため CP/M 通常動作時には実害はないが、設計上の非対称性であり、初期化前に呼ばれた場合のリスクがある。

**影響**: 初期化前の意図しない割り込み送信。低リスク。

---

### BUG #6 [CRITICAL]: `LOAD_CCP_BDOS` にリジェクション・リトライがない

**ファイル**: `z80/cpm22/bios/bios.asm` (L341-344)

**問題のコード**:

```asm
; bios.asm L342
        CALL DISK_READ_SUB
        OR A
        RET Z                   ; Success
        ; ↓ A≠0 → 全てエラー扱い → BOOT_ERROR_HALT
```

通常の READ パス (`RETRY_READ`, L665) では `CP 4` によりリジェクト(4)をリトライするが、`LOAD_CCP_BDOS` では `OR A` で非ゼロをすべてエラーとして `BOOT_ERROR_HALT` に到達する。

**`DISK_READ_SUB` の返すステータス**:

| 値 | 意味 | 通常 READ (MISHIT_CACHE) の処理 | LOAD_CCP_BDOS の処理 |
|----|------|-------------------------------|---------------------|
| 0 | 成功 | 成功 | 成功 |
| 1 | 読み込み中 | （通常到達しない） | **HALT!** |
| 2 | エラー | READ_ERROR | **HALT!** |
| 4 | リジェクト | **リトライ** | **HALT!** |

**AVR 側でリジェクトが発生する条件** (`em_diskio.c`, `OUT_13_DSK_Read()`):

```c
void OUT_13_DSK_Read(uint8_t data)
{
    // WRITE が進行中なら REJECT
    if (cfd->write.state == DOING || cfd->write.state == REQUESTING) {
        cfd->read.state = REJECTED;
    } else {
        switch (cfd->read.state) {
        case REQUESTING:  // 前回の READ が未完了
        case DOING:
            cfd->read.state = REJECTED;
            break;
        ...
        }
    }
}
```

**BOOT_ERROR_HALT への到達シナリオ**:

- **シナリオ 1: SD カードのトランジェントエラー** — 多数のディスクアクセス中に SD カードに一時的なエラー（CRC エラー、タイムアウト等）が発生。BDOS がエラー検出後にウォームブートを実行するが、`LOAD_CCP_BDOS` では SD カードエラーが継続するとステータス 2 を受けて即 HALT に到達する。
- **シナリオ 2: BUG #1 → BUG #7 → BUG #6 の連鎖** — 割り込み消失による BDOS エラー → WBOOT → SELDSK が進行中操作を強制リセット → FatFs 状態不整合 → HALT。後述「3.2 最も可能性の高い障害シナリオ」を参照。

**影響**: WBOOT 時のリジェクト発生でシステムが HALT する。

---

### BUG #7 [CRITICAL]: `SELDSK` が AVR 側の進行中ディスク操作を強制リセット

**ファイル**: `avr/src/emuldev/em_diskio.c` (L94-119)

**問題のコード**:

```c
// em_diskio.c L114
cfd->read.state   = IDLE;
cfd->read.result  = FR_OK;
cfd->write.state  = IDLE;
cfd->write.result = FR_OK;
```

`OUT_0A_DSK_SelectDisk()` は I/O 完了確認なしに read/write 状態を IDLE にリセットする。Timer2 ISR で `em_disk_write()` が `f_write()` 実行中であっても、状態だけが IDLE に戻り FatFs オブジェクトが不整合状態になる。

**影響**: FatFs 状態破壊、後続のディスク操作でのエラーまたはデータ破損。

**競合の具体的タイムライン** (BUG #1 起因の WBOOT 発生時):

```text
時刻    Z80 側                    AVR 側
──────────────────────────────────────────────────
T+0ms   Zork I: WRITE 要求       em_disk_write: state=REQUESTING
T+10ms                            Timer2: em_disk_write 開始
                                  state=DOING, f_write() 実行中...
T+12ms  BDOS エラー →
        WBOOT 開始
T+15ms  SELDSK(A:) →
        OUT(PORT_SELDSK)          OUT_0A_DSK_SelectDisk:
                                  write.state = IDLE (★強制リセット!)
                                  ※ f_write() はまだ実行中!
T+16ms  LOAD_CCP_BDOS:
        OUT(PORT_DSKRD)           OUT_13_DSK_Read:
                                  write.state=IDLE → read.state=REQUESTING
T+20ms                            Timer2: em_disk_read 開始
                                  state=REQUESTING → DOING → f_read()
                                  ※ 同じ FIL オブジェクトで f_write() と並行実行!
                                  ※ FatFs は非リエントラント → 破損!
T+25ms                            f_read() エラー or ゴミデータ返却
                                  → 割り込み送信
T+26ms  DISK_READ_SUB:
        ステータス=2(エラー)
        LOAD_CCP_BDOS: OR A → NZ
        → BOOT_ERROR_HALT!        ★ Z80 HALT
```

この競合は `OUT_0A_DSK_SelectDisk()` が `write.state` を強制的に IDLE にリセットする（実際の SD カード書き込みの完了を待たない）ことに起因する。FatFs は非リエントラントであるため、同一の FIL オブジェクトに対する `f_read()` と `f_write()` の並行実行はファイルシステムの状態破壊を引き起こす。

---

### BUG #8 [IMPORTANT]: Timer2 ISR (`ISR_NOBLOCK`) の再入リスク

**ファイル**: `avr/src/isr.c` (L93-99)

**問題のコード**:

```c
// isr.c L93
ISR(TIMER2_COMP_vect, ISR_NOBLOCK)
{
    disk_timerproc();
    em_disk_read();
    em_disk_write();
    em_led_heartbeat(2);
}
```

`ISR_NOBLOCK` により ISR 突入直後に `sei()` が挿入される。SD カード操作が 10ms 以上かかると次の Timer2 割り込みで再入が発生する。状態マシンにより二重処理は抑止されるが、ISR ネストによるスタック消費は蓄積する。

**影響**: AVR スタックオーバーフロー（SRAM 4KB 制約下）。

**具体例**: SD カードが一時的に 100ms の応答遅延を生じた場合、Timer2（10ms 周期）が最大 10 回再入し、各 ISR フレームのスタック消費が蓄積する。ATmega128 の SRAM は 4KB であり、グローバル変数・ヒープと共有するため、スタック余裕は限られる。

---

### BUG #9 [IMPORTANT]: DEBUGGER の HALT が DI 状態で実行される

**ファイル**: `z80/cpm22/bios/bios.asm` (L92-95)

**問題のコード**:

```asm
; bios.asm L92
        LD SP, (SP_ADR)         ; Restore SP
;        EI                      ; ★ コメントアウト
        HALT                    ; Wait for INT 4
        RET                     ; Resume
```

Z80 が DI 状態で HALT に入ると、INT による HALT 解除が不可能。NMI またはリセットでしか復帰できない。`dbg_Continue()` は `Z80_EXTINT_low()` で INT を送るが、DI 状態では Z80 に届かない。

**影響**: デバッガの continue 機能が動作しない。

**到達経路**: RST 7（オペコード 0xFF）により `DEBUGGER` に到達する。BIOS のベクタ設定:

```asm
0x0038: JP DEBUGGER          ; RST 7 ハンドラ
```

通常のコード実行では RST 7 は発生しないが、以下の場合に到達し得る:

1. メモリ破損（DMA 書き込みの不整合、FatFs 競合等）により TPA のコードが破壊され、Z80 の PC が不正なアドレスに飛ぶ
2. メモリ上の 0xFF バイトをオペコードとして実行 → RST 7 → DEBUGGER
3. DI → HALT → **永久停止**（NMI またはリセットでしか復帰不可）

---

### BUG #10 [MODERATE]: `int_level_write`/`int_level_read` に `volatile` がない

**ファイル**: `avr/src/emuldev/em_diskio.c` (L22-23)

**問題のコード**:

```c
// em_diskio.c L22
static uint8_t int_level_write = 128;
static uint8_t int_level_read  = 128;
```

INT1 コンテキスト（OUT ハンドラ `OUT_0F_DSK_WriteIntLevel` / `OUT_14_DSK_ReadIntLevel`）で書き込まれ、Timer2 コンテキスト（`em_disk_read()` / `em_disk_write()`）で読み込まれる。`volatile` がなくコンパイラ最適化でレジスタキャッシュされるリスクがある。

同ファイル内の他の `em_consoleio.c` では `z80_int_num_rx1` / `z80_int_num_tx1` が `volatile` 宣言されており、非対称。

**影響**: コンパイラ最適化条件下で割り込み通知の欠落。

---

### BUG #11 [MODERATE]: `IN_0E_DSK_WriteStatus()`/`IN_13_DSK_ReadStatus()` の `sei()` 問題

**ファイル**: `avr/src/emuldev/em_diskio.c` (L202-223, L468-489)

**問題のコード**:

```c
// em_diskio.c L202
uint8_t IN_0E_DSK_WriteStatus()
{
    cli();
    uint8_t st = 0x00;
    switch (cfd->write.state) {
    ...
    }
    sei();          // ← INT0 ISR コンテキストで sei() を呼ぶ
    return st;
}
```

この関数は INT0 ISR（Z80 IN 命令ハンドラ）から呼ばれる。INT0 ISR は通常 ISR（割り込み無効）であるため、内部の `cli()`/`sei()` は不要であり、かつ `sei()` が ISR 完了前に割り込みを再有効化する。BUG #4 と同じパターン。

**影響**: INT0 ISR 内での予期しない割り込み再有効化。BUG #1 のレース窓を間接的に広げる可能性がある。

---

### BUG #12 [MODERATE]: `Enqueue_RX1_Buf()` — バッファ満杯時のデータロストと偽割り込み

**ファイル**: `avr/src/emuldev/em_consoleio.c` (L43-53)

**問題のコード**:

```c
// em_consoleio.c L43
void Enqueue_RX1_Buf()
{
    while (UCSR1A & _BV(RXC1)) {
        x_enqueue(&cb_rx1, UDR1);          // ← 戻り値を無視
        if (z80_int_num_rx1 < 128) {
            Z80_EXTINT_low(z80_int_num_rx1 << 1);  // ← 失敗時も実行
        }
    }
}
```

**原因**: `cb_rx1` が満杯（8 バイト）のとき、`x_enqueue()` は 3 を返してデータを破棄する。しかし `UDR1` は引数渡しの時点で既に UART レジスタから読み出されており、受信データは失われる。さらに `Z80_EXTINT_low()` が呼ばれ、Z80 に ISR_03（CONIN 通知）が送られるが、新規データはバッファに入っていないため偽の割り込み通知となる。

**影響**:

- 受信文字のロスト。RX バッファが 8 バイトと小さいため、Z80 が CONIN 読み出しに遅延がある場合に発生しうる。
- 偽割り込みが BUG #1 のレース窓を広げる。

**補足 — RX バッファサイズの問題**: `RX1_BUF_SIZE` が 8 バイトしかないため、高速タイピングやペースト操作で Z80 側の CONIN 読み出しが追いつかない場合にバッファ満杯が頻発し、本バグの影響が拡大する。戻り値チェック（偽割り込み防止）に加え、`RX1_BUF_SIZE` を 32〜64 バイトに拡大することでバッファ溢れ自体の発生頻度を低減すべきである。修正方法は Step 6 を参照。

---

### BUG #13 [MODERATE]: `ConsoleBuffer` の `count`/`head`/`tail` に `volatile` がない

**ファイル**: `avr/src/xconsoleio.h`

**問題のコード**:

```c
typedef struct {
    char* buffer;
    int size;
    int head;    // ← volatile なし
    int tail;    // ← volatile なし
    int count;   // ← volatile なし
} ConsoleBuffer;
```

**原因**: `cb_rx1` は USART1_RX ISR（`Enqueue_RX1_Buf` → `x_enqueue`）と INT0 ISR（`IN_00_CONIN` → `x_dequeue`、`IN_01_CONIN_GetStatus` → `count` 直読み）の複数コンテキストから参照される。`cb_tx1` は INT1 ISR（`OUT_05_CONOUT` → `x_enqueue`）と Timer0 ISR（`Transmit_TX1_Buf` → `x_dequeue`）から参照される。

`IN_01_CONIN_GetStatus()` / `IN_06_CONOUT_GetStatus()` は `count` を `cli()` なしで直接読む：

```c
uint8_t IN_01_CONIN_GetStatus() {
    return cb_rx1.count & 0xff;
}
```

`volatile` がないため、コンパイラが `count` をレジスタにキャッシュし、ISR からの更新が反映されない可能性がある。BUG #10（`int_level_read/write`）と同じパターン。

**補足**: 現在のバッファサイズ（8, 128）は 8 ビット幅に収まるが、`count` は `int` 型（16 ビット）であるため、AVR の 8 ビットアーキテクチャでは非アトミック読み出しとなる。現行サイズでは上位バイトが常に 0 のため torn read の実害は出にくい。

**影響**: コンパイラ最適化条件下で、Z80 へのバッファステータス報告が不正確になる。

---

### BUG #14 [LOW]: `USART1_Transmit()` の Timer0 ISR 内ビジーウェイト

**ファイル**: `avr/src/usart.c` (`USART1_Transmit`), `avr/src/emuldev/em_consoleio.c` (`Transmit_TX1_Buf`), `avr/src/isr.c` (Timer0 ISR)

**問題のコード**:

```c
// usart.c
void USART1_Transmit(uint8_t data)
{
    while (!(UCSR1A & _BV(UDRE1)));   // ← UART TX バッファ空き待ち
    UDR1 = data;
}
```

**原因**: `Transmit_TX1_Buf()` は Timer0 ISR（1ms 周期、通常 ISR）から呼ばれ、`USART1_Transmit()` を通じて UART TX バッファ空きをビジーウェイトする。Timer0 は通常 ISR のため、実行中は全割り込みが無効になる。

ATmega128 の USART はダブルバッファリング構造（UDR + シフトレジスタ）を持つため、実際のブロック時間は 1 文字送信時間より大幅に短い。UDR へ書き込まれたデータは前のシフトレジスタ送信完了後にすぐ転送され、UDR が空になる。9600 bps（1 文字 ≈ 1.04ms）で Timer0 周期（1ms）とのずれが蓄積し、約 52 文字に 1 回、~40µs のブロックが発生する。19200 bps では文字送信時間（~0.52ms）が Timer0 周期より十分短いためブロックは発生しない。

| ボーレート | 最大ブロック時間 | 発生頻度 |
|-----------|----------------|---------|
| 9600 bps  | ~40µs          | ~52 文字に 1 回 |
| 19200 bps | 0              | なし |

**影響**: ブロック中は全割り込みが遅延するが、ダブルバッファリングにより実際のブロック時間は短い。ただし ISR 内でのビジーウェイトは原則として避けるべき設計パターンであり、非ブロッキング化が望ましい。

---

### BUG #15 [LOW]: CONOUT フロー制御の閾値ギャップ

**ファイル**: `avr/src/emuldev/em_consoleio.c` (`Transmit_TX1_Buf`), `z80/cpm22/bios/bios.asm` (`CONOUT`)

**問題のコード**:

Z80 CONOUT（bios.asm）:

```asm
CONOUT:
        PUSH AF
        DI
        IN A, (PORT_CONOUT_STS)     ; cb_tx1.count を取得
        CP 64                        ; count >= 64 で待機開始
        JR C, CONOUT2
        XOR A
        LD (IS_CONOUT_FREE), A       ; フラグクリア
        EI
CONOUT1:
        LD A, (IS_CONOUT_FREE)       ; INT 2 通知を待つ
        OR A
        JR Z, CONOUT1
```

AVR Transmit_TX1_Buf（em_consoleio.c）:

```c
if (cb_tx1.count == 0 ||
    cb_tx1.count == cb_tx1.size / 4 ||    //  32
    cb_tx1.count == cb_tx1.size / 2) {    //  64
    Z80_EXTINT_low(z80_int_num_tx1 << 1); // INT 2
}
```

**原因**: Z80 は `count >= 64` で送信待ちに入るが、AVR の通知は `count` が**ちょうど** 0, 32, 64 に一致したときのみ送られる。`count` が 64 から 63 に減少した瞬間には通知が発生しない。Z80 が `count = 64` を読み取り待機に入った場合、`count` が 32 に到達するまで通知を受けられず、32ms（9600 bps 運用時）の不要な待ちが発生する。

**影響**: バースト出力（DIR, TYPE 等）時のスループット低下。インタラクティブ操作（1 文字エコー）では影響軽微。

---

## 3. バグ間の依存関係

```text
BUG #4  ────拡大────→ BUG #1 のレース窓
BUG #11 ────拡大────→ BUG #1 のレース窓
BUG #5  ────拡大────→ BUG #1 のレース窓（軽微）
BUG #12 ────拡大────→ BUG #1 のレース窓（偽割り込み）
BUG #10 ────────────→ 割り込み通知欠落 → BUG #1 と類似の影響
BUG #13 ────────────→ ステータス不正 → BUG #1 と類似の影響

BUG #1  ────────────→ 割り込み消失 → Z80 ポーリング永久滞留
BUG #1  ────連鎖────→ BUG #7 → BUG #6 → HALT（仮説）

BUG #8  ────────────→ AVR スタックオーバーフロー → 未定義動作
BUG #8  ────増幅────→ BUG #2 のデッドロック発生条件（Timer2 再入で cli() 区間に到達）

BUG #2  ────────────→ AVR/Z80 デッドロック

BUG #6  ────────────→ BOOT_ERROR_HALT（直接到達）
BUG #7  ────────────→ FatFs 状態破壊 → BUG #6 のトリガー

BUG #14 ────────────→ ISR 内ビジーウェイト（軽微、ダブルバッファで緩和）
BUG #15 ────────────→ CONOUT スループット低下

BUG #9  ────────────→ デバッガ復帰不能（独立）
```

### 依存グループの整理

**グループ A: 割り込みレース関連**

- 根本: BUG #1
- 窓拡大: BUG #4, BUG #11, BUG #5, BUG #12
- 補足: BUG #10, BUG #13

**グループ B: ディスク I/O 信頼性関連**

- BUG #6（リトライ欠如）
- BUG #7（SELDSK 状態破壊）
- BUG #8（Timer2 再入）

**グループ C: デッドロック関連**

- BUG #2（`cli()` + `ExtMem_attach()`）
- BUG #8 が増幅因子

**グループ D: コンソール I/O 性能関連**

- BUG #14（Timer0 ISR ブロッキング）
- BUG #15（閾値ギャップ）

**グループ E: 独立**

- BUG #9（デバッガ）

### 3.2 最も可能性の高い障害シナリオ

Z80 の `/HALT` ピン（PB7）が LOW になる（Z80 が真の HALT 命令を実行する）最も可能性の高いシナリオを以下に示す。

#### BIOS 内の HALT 命令の所在

| アドレス | コンテキスト | 割り込み状態 | 到達条件 |
|----------|-------------|-------------|---------|
| 0xF292 | DEBUGGER ハンドラ | DI（無効） | RST 7 (0x0038 → JP DEBUGGER) が実行された場合 |
| 0xF436 | BOOT_ERROR_HALT | EI（有効） | LOAD_CCP_BDOS 内の DISK_READ_SUB が失敗した場合 |

BIOS コードに他の HALT 命令は存在しない。CCP/BDOS は標準 CP/M 2.2 であり通常 HALT を含まない。

#### 推定される障害連鎖

```text
 ① BUG #1 (z80_int_vector レースコンディション)
    により DISK I/O 完了割り込みが消失
         ↓
 ② Z80 が WAIT_READ_COMPLETE ビジーループに滞留
    （この時点では HALT ではない）
         ↓
 ③ ユーザーから見るとシステムは無応答
    一定時間後、何らかの割り込みイベントにより
    Z80 が IS_READ_DONE=1 を誤検知するか、
    または BDOS レベルのタイムアウト/エラー発生
         ↓
 ④ BDOS がディスクエラーを検出し、
    ウォームブート(WBOOT)を実行
         ↓
 ⑤ WBOOT → LOAD_CCP_BDOS →
    SELDSK(A:) が AVR 側の進行中操作を強制リセット (BUG #7)
         ↓
 ⑥ DISK_READ_SUB が リジェクト(4) または エラー(2) を返す
    (SD カードの一時的エラー、または FatFs 状態不整合)
         ↓
 ⑦ LOAD_CCP_BDOS にリトライがないため (BUG #6)
    即座に BOOT_ERROR_HALT (0xF436) に到達
         ↓
 ⑧ Z80 が HALT 命令を実行 → /HALT = LOW
    "System HALT due to CCP+BDOS load error." をコンソールに出力
    （バッファリングのため表示が見えない可能性あり）
```

この連鎖は依存関係グラフの `BUG #1 ────連鎖────→ BUG #7 → BUG #6 → HALT` に対応する。BUG #8（Timer2 再入）が AVR スタックオーバーフローを引き起こした場合も、同様の経路で HALT に到達し得る。

#### 代替経路: TPA 内の 0x76 バイト実行

Z80 の PC が何らかの原因（スタック破壊、DMA によるコード領域の部分的上書き等）で不正なアドレスに飛び、メモリ上のデータバイト 0x76（HALT のオペコード）を命令として実行する可能性もある。ただし、Mode 2 割り込みベクタテーブル（0xF300 ページ）の正規ベクタバイトは 0, 2, 4, 6, 8 のいずれかであり、BUG #1 のレースコンディションではこれらの値間の入れ替わりしか発生しないため、不正なアドレスへのジャンプは BUG #1 単独では起きにくい。FatFs 競合によるメモリ破損（BUG #7）が主要な要因と考えられる。

---

## 4. 修正の優先順位

| 優先度 | BUG ID | 修正難易度 | Phase | 根拠 |
|--------|--------|-----------|-------|------|
| 1 | BUG #4 | 低 | Phase 1 | レース窓を即座に縮小。BUG #1 修正の前提 |
| 2 | BUG #11 | 低 | Phase 1 | BUG #4 と同じパターン。同時に修正可能 |
| 3 | BUG #10 | 低 | Phase 1 | 宣言の修正のみ。割り込み通知欠落を防止 |
| 4 | BUG #13 | 低 | Phase 1 | BUG #10 と同パターン。コンソール側の volatile 追加 |
| 5 | BUG #5 | 低 | Phase 1 | 1 行のガード追加。設計の一貫性を確保 |
| 6 | BUG #12 | 低 | Phase 1 | 戻り値チェック追加。偽割り込み防止 |
| 7 | BUG #6 | 低 | Phase 1 | HALT 到達を直接防止。Z80 側 3 行の追加 |
| 8 | BUG #8 | 低 | Phase 1 | スタックオーバーフロー防止。BUG #2 修正の前提 |
| 9 | BUG #9 | 低 | Phase 1 | コメント解除のみ |
| 10 | BUG #14 | 低 | Phase 1 | ISR 内ビジーウェイト除去 |
| 11 | BUG #15 | 低 | Phase 1 | 通知閾値修正。CONOUT スループット改善 |
| 12 | BUG #1 | 中 | Phase 2 | 割り込み消失の根本対策 |
| 13 | BUG #2 | 中 | Phase 2 | デッドロック防止 |
| 14 | BUG #7 | 高 | Phase 3 | アーキテクチャの見直しを伴う |

---

## 5. 修正ステップ

### Phase 1: 低リスク・独立した修正（Step 1〜11）

Phase 1 の各ステップは互いに独立しており、任意の順で適用可能。ただし Step 5（BUG #5）、Step 7（BUG #14）、Step 8（BUG #15）は同一関数 `Transmit_TX1_Buf()` を修正するため、すべて適用する場合は統合コードを確認すること。

---

#### Step 1: BUG #4 — `x_enqueue()`/`x_dequeue()` の SREG 保存/復元

**対象ファイル**: `avr/src/xconsoleio.c`

**修正方針**: `cli()`/`sei()` を `SREG` 保存/復元に変更し、呼び出し元の割り込み状態を破壊しないようにする。

**現行コード** (L143-161):

```c
int x_enqueue(ConsoleBuffer* cb, char data)
{
    if (cb->count == cb->size) {
        return 3;
    }

    cli();
    cb->buffer[cb->tail] = data;
    cb->tail = (cb->tail + 1) % cb->size;
    cb->count++;
    sei();
    ...
}
```

**修正後コード**:

```c
int x_enqueue(ConsoleBuffer* cb, char data)
{
    if (cb->count == cb->size) {
        return 3;
    }

    uint8_t sreg = SREG;
    cli();
    cb->buffer[cb->tail] = data;
    cb->tail = (cb->tail + 1) % cb->size;
    cb->count++;
    SREG = sreg;
    ...
}
```

**同様に `x_dequeue()`** (L164-176):

```c
char x_dequeue(ConsoleBuffer* cb)
{
    if (cb->count == 0) {
        return '\0';
    }

    uint8_t sreg = SREG;
    cli();
    char data = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % cb->size;
    cb->count--;
    SREG = sreg;
    return data;
}
```

**同様に `x_flush()`** にも同じパターンを適用する。`initConsoleBuffer()` は初期化時のみ呼ばれるため `sei()` のままでもよいが、統一する場合は同様に修正する。

**検証方法**: コンパイル確認。既存動作に影響しないことを確認（通常コンテキストでは `SREG` 復元は事実上 `sei()` と同等）。

---

#### Step 2: BUG #11 — `IN_0E_DSK_WriteStatus()`/`IN_13_DSK_ReadStatus()` の SREG 保存/復元

**対象ファイル**: `avr/src/emuldev/em_diskio.c`

**修正方針**: Step 1 と同じパターンで `cli()`/`sei()` を SREG 保存/復元に変更する。

**修正対象 1** — `IN_0E_DSK_WriteStatus()` (L202-223):

```c
uint8_t IN_0E_DSK_WriteStatus()
{
    uint8_t sreg = SREG;
    cli();
    uint8_t st = 0x00;
    switch (cfd->write.state) {
        case IDLE:
            if (cfd->write.result != FR_OK) {
                st = 0x02;
            }
            break;
        case REQUESTING:
        case DOING:
            st = 0x01;
            break;
        case REJECTED:
            st = 0x04;
            break;
    }
    SREG = sreg;
    return st;
}
```

**修正対象 2** — `IN_13_DSK_ReadStatus()` (L468-489):

```c
uint8_t IN_13_DSK_ReadStatus()
{
    uint8_t sreg = SREG;
    cli();
    uint8_t st = 0x00;
    switch (cfd->read.state) {
    case IDLE:
        if (cfd->read.result != FR_OK) {
            st = 0x02;
        }
        break;
    case REQUESTING:
    case DOING:
        st = 0x01;
        break;
    case REJECTED:
        st = 0x04;
        break;
    }
    SREG = sreg;
    return st;
}
```

**検証方法**: コンパイル確認。

---

#### Step 3: BUG #10 — `int_level_write`/`int_level_read` に `volatile` を追加

**対象ファイル**: `avr/src/emuldev/em_diskio.c`

**修正方針**: INT1 と Timer2 の異なる ISR コンテキスト間で共有される変数に `volatile` を付与する。

**現行コード** (L22-23):

```c
static uint8_t int_level_write = 128;
static uint8_t int_level_read  = 128;
```

**修正後コード**:

```c
static volatile uint8_t int_level_write = 128;
static volatile uint8_t int_level_read  = 128;
```

**検証方法**: コンパイル確認。生成されるコードが変数をメモリから毎回読むようになることを逆アセンブルで確認可能。

---

#### Step 4: BUG #13 — `ConsoleBuffer` の `volatile` 追加

**対象ファイル**: `avr/src/xconsoleio.h`

**修正方針**: ISR 間で共有される `ConsoleBuffer` のフィールドに `volatile` を付与する。

**現行コード**:

```c
typedef struct {
    char* buffer;
    int size;
    int head;
    int tail;
    int count;
} ConsoleBuffer;
```

**修正後コード**:
```c
typedef struct {
    char* buffer;
    int size;
    volatile int head;
    volatile int tail;
    volatile int count;
} ConsoleBuffer;
```

**検証方法**: コンパイル確認。`IN_01_CONIN_GetStatus()` / `IN_06_CONOUT_GetStatus()` が正しいバッファ残量を返すことを確認。

---

#### Step 5: BUG #5 — `Transmit_TX1_Buf()` にガードを追加

**対象ファイル**: `avr/src/emuldev/em_consoleio.c`

**修正方針**: `Enqueue_RX1_Buf()` と同様に、`Z80_EXTINT_low()` 呼び出し前に `z80_int_num_tx1 < 128` をチェックする。

**現行コード** (L52-64):

```c
void Transmit_TX1_Buf(void)
{
    char data = x_dequeue(&cb_tx1);
    if (data != '\0') {
        USART1_Transmit(data);
        if (cb_tx1.count == 0 ||
            cb_tx1.count == cb_tx1.size / 4 ||
            cb_tx1.count == cb_tx1.size / 2) {
            Z80_EXTINT_low(z80_int_num_tx1 << 1);
        }
    }
}
```

**修正後コード**:

```c
void Transmit_TX1_Buf(void)
{
    char data = x_dequeue(&cb_tx1);
    if (data != '\0') {
        USART1_Transmit(data);
        if (z80_int_num_tx1 < 128 &&
            (cb_tx1.count == 0 ||
             cb_tx1.count == cb_tx1.size / 4 ||
             cb_tx1.count == cb_tx1.size / 2)) {
            Z80_EXTINT_low(z80_int_num_tx1 << 1);
        }
    }
}
```

**検証方法**: コンパイル確認。BOOT 後は `z80_int_num_tx1 = 2` のため動作に変化なし。

---

#### Step 6: BUG #12 — `Enqueue_RX1_Buf()` の戻り値チェック追加

**対象ファイル**: `avr/src/emuldev/em_consoleio.c`

**修正方針**: `x_enqueue()` の戻り値を確認し、バッファ満杯でエンキュー失敗した場合は Z80 への割り込み通知を抑止する。

**現行コード** (L43-53):

```c
void Enqueue_RX1_Buf()
{
    while (UCSR1A & _BV(RXC1)) {
        x_enqueue(&cb_rx1, UDR1);
        if (z80_int_num_rx1 < 128) {
            Z80_EXTINT_low(z80_int_num_rx1 << 1);
        }
    }
}
```

**修正後コード**:

```c
void Enqueue_RX1_Buf()
{
    while (UCSR1A & _BV(RXC1)) {
        int st = x_enqueue(&cb_rx1, UDR1);
        if (st != 3 && z80_int_num_rx1 < 128) {
            Z80_EXTINT_low(z80_int_num_rx1 << 1);
        }
    }
}
```

**補足**: `x_enqueue()` が 3 を返すのはバッファ満杯時であり、データは破棄される。この場合 Z80 への通知を送っても読み出せるデータが増えていないため、通知は不要。

**追加修正 — RX バッファサイズの拡大**:

`RX1_BUF_SIZE` を 8 → 64 バイトに拡大し、バッファ溢れ自体の発生頻度を低減する。

**対象ファイル**: `avr/src/emuldev/em_consoleio.c`

現行コード (L14):

```c
#define RX1_BUF_SIZE	8
```

修正後コード:

```c
#define RX1_BUF_SIZE	64
```

SRAM 消費は 56 バイト増加するが、ATmega128 の 4KB SRAM では許容範囲内。

**検証方法**: コンパイル確認。高速ペースト操作での文字ロスト低減を確認。

---

#### Step 7: BUG #14 — `Transmit_TX1_Buf()` の非ブロッキング化

**対象ファイル**: `avr/src/emuldev/em_consoleio.c`

**修正方針**: `USART1_Transmit()` を呼ぶ代わりに、UDRE1 フラグを事前チェックし、TX ビジーなら送信をスキップして次の Timer0 サイクルに持ち越す。

**現行コード** (L63-75):

```c
void Transmit_TX1_Buf(void)
{
    char data = x_dequeue(&cb_tx1);
    if (data != '\0') {
        USART1_Transmit(data);
        if (cb_tx1.count == 0 ||
            cb_tx1.count == cb_tx1.size / 4 ||
            cb_tx1.count == cb_tx1.size / 2) {
            Z80_EXTINT_low(z80_int_num_tx1 << 1);
        }
    }
}
```

**修正後コード**:

```c
void Transmit_TX1_Buf(void)
{
    // UART TX ビジーなら次の Timer0 サイクルまでスキップ
    if (!(UCSR1A & _BV(UDRE1))) return;

    char data = x_dequeue(&cb_tx1);
    if (data != '\0') {
        UDR1 = data;    // UDRE1 確認済のため直接書き込み（ブロックなし）
        if (cb_tx1.count == 0 ||
            cb_tx1.count == cb_tx1.size / 4 ||
            cb_tx1.count == cb_tx1.size / 2) {
            Z80_EXTINT_low(z80_int_num_tx1 << 1);
        }
    }
}
```

**スループットへの影響**:

ATmega128 の USART ダブルバッファリングにより、通常は UDRE1 が既にセットされているため dequeue→送信が即座に行われる。9600 bps ではドリフトにより ~52 文字に 1 回 dequeue がスキップされるが、全体スループットへの影響は微小。

| ボーレート | 修正前 (最大) | 修正後 (最大) | 備考 |
|-----------|-------------|-------------|------|
| 9600 bps | ~960 chars/sec | ~940 chars/sec | ~52 文字に 1 回スキップ |
| 19200 bps | ~1000 chars/sec | ~1000 chars/sec | 変化なし |

**検証方法**: コンパイル確認。9600 bps でのコンソール出力動作確認。

---

#### Step 8: BUG #15 — CONOUT 通知閾値の修正

**対象ファイル**: `avr/src/emuldev/em_consoleio.c`

**修正方針**: Z80 CONOUT の送信待ち閾値（`count >= 64 = cb_tx1.size / 2`）を下回った直後に通知が送られるよう、閾値を修正する。

**現行コード** (`Transmit_TX1_Buf` 内の通知条件):

```c
if (cb_tx1.count == 0 ||
    cb_tx1.count == cb_tx1.size / 4 ||
    cb_tx1.count == cb_tx1.size / 2) {
    Z80_EXTINT_low(z80_int_num_tx1 << 1);
}
```

**修正後コード**:

```c
if (cb_tx1.count == 0 ||
    cb_tx1.count == cb_tx1.size / 4 ||
    cb_tx1.count == cb_tx1.size / 2 - 1) {
    Z80_EXTINT_low(z80_int_num_tx1 << 1);
}
```

**変更内容**: `cb_tx1.size / 2`（= 64）→ `cb_tx1.size / 2 - 1`（= 63）。Z80 CONOUT は `count >= 64` で待機に入るため、`count = 63`（バッファが半分以下になった最初の瞬間）で通知を送ることで、不要な待ち時間（最大 32ms）を解消する。

**検証方法**: コンパイル確認。CONOUT バースト出力（DIR, TYPE）時のレスポンス改善を確認。

---

#### Step 9: BUG #6 — `LOAD_CCP_BDOS` にリジェクション・リトライを追加

**対象ファイル**: `z80/cpm22/bios/bios.asm`

**修正方針**: `MISHIT_CACHE` の `RETRY_READ`（L665-670）と同じリトライロジックを `LOAD_CCP_BDOS` に適用する。

**現行コード** (L341-344):

```asm
        ; Load CPP+BDOS
        CALL DISK_READ_SUB
        OR A
        RET Z                           ; Success
```

**修正後コード**:

```asm
        ; Load CPP+BDOS
LOAD_RETRY:
        CALL DISK_READ_SUB
        CP 4
        JR Z, LOAD_RETRY               ; retry if rejected
        OR A
        RET Z                           ; Success
```

**検証方法**: アセンブル・リンク確認。BIOS イメージサイズが増えるため、アドレスマップへの影響を確認。

---

#### Step 10: BUG #8 — Timer2 ISR に再入ガードを追加

**対象ファイル**: `avr/src/isr.c`

**修正方針**: `static volatile` フラグで再入を検知し、既に実行中なら即 return する。

**現行コード** (L93-99):

```c
ISR(TIMER2_COMP_vect, ISR_NOBLOCK)
{
    disk_timerproc();
    em_disk_read();
    em_disk_write();
    em_led_heartbeat(2);
}
```

**修正後コード**:

```c
ISR(TIMER2_COMP_vect, ISR_NOBLOCK)
{
    static volatile bool in_progress = false;
    if (in_progress) return;
    in_progress = true;

    disk_timerproc();
    em_disk_read();
    em_disk_write();
    em_led_heartbeat(2);

    in_progress = false;
}
```

**注意**: `disk_timerproc()` は FatFs のタイマ管理（10ms カウンタ）であり、再入ガードで呼び出しがスキップされるとタイムアウト計算に誤差が生じる可能性がある。ただし、SD カード操作が 10ms 超の場合にのみスキップされるため、実用上問題にならない。必要であれば `disk_timerproc()` のみガードの外に出すことも可能：

```c
ISR(TIMER2_COMP_vect, ISR_NOBLOCK)
{
    static volatile bool in_progress = false;
    disk_timerproc();
    if (in_progress) return;
    in_progress = true;

    em_disk_read();
    em_disk_write();
    em_led_heartbeat(2);

    in_progress = false;
}
```

**検証方法**: コンパイル確認。長時間動作テストでスタック消費が安定することを確認。

---

#### Step 11: BUG #9 — DEBUGGER の `EI` コメント解除

**対象ファイル**: `z80/cpm22/bios/bios.asm`

**修正方針**: コメントアウトされている `EI` を有効化する。

**現行コード** (L92-95):

```asm
        LD SP, (SP_ADR)         ; Restore SP
;        EI
        HALT                    ; Wait for INT 4
        RET                     ; Resume
```

**修正後コード**:

```asm
        LD SP, (SP_ADR)         ; Restore SP
        EI
        HALT                    ; Wait for INT 4
        RET                     ; Resume
```

**検証方法**: アセンブル確認。TinyMonitor から `dbg_Continue()` でブレーク復帰が動作することを確認。

---

### Phase 2: 根本原因の修正（Step 12〜13）

Phase 1 完了後に着手する。

---

#### Step 12: BUG #1 — `z80_int_vector` のキュー化

**対象ファイル**: `avr/src/z80io.c`, `avr/src/z80io.h`, `avr/src/isr.c`

**修正方針**: 割り込みベクタを FIFO キューで管理し、INT4 ISR でキューから順次消費する。

**設計**:

1. リングバッファ型のキュー（8 エントリ）を追加
2. `Z80_EXTINT_low()` → キューにベクタを追加し、キュー長が 1 になったときのみ `/INT=Low` にする
3. `ISR(INT4_vect)` → キューからベクタを消費し、残りがあれば再度 `/INT=Low` にする

**追加コード** (`z80io.c`):

```c
#define INT_QUEUE_SIZE 8
static volatile uint8_t int_queue[INT_QUEUE_SIZE];
static volatile uint8_t int_queue_head = 0;
static volatile uint8_t int_queue_tail = 0;

void Z80_EXTINT_enqueue(uint8_t vector)
{
    uint8_t sreg = SREG;
    cli();
    uint8_t next = (int_queue_tail + 1) % INT_QUEUE_SIZE;
    if (next != int_queue_head) {
        int_queue[int_queue_tail] = vector;
        int_queue_tail = next;
    }
    // キューに 1 個だけ入った場合（直前が空だった場合）のみ /INT をアサート
    uint8_t count = (int_queue_tail - int_queue_head + INT_QUEUE_SIZE) % INT_QUEUE_SIZE;
    if (count == 1) {
        z80_int_vector = vector;
        CLR_BIT(PORTD, PORTD4);     // /INT = Low
    }
    SREG = sreg;
}
```

**INT4 ISR の修正** (`isr.c`):

```c
ISR(INT4_vect)
{
    // キューからベクタを取り出す
    uint8_t vector = z80_int_vector;
    if (int_queue_head != int_queue_tail) {
        int_queue_head = (int_queue_head + 1) % INT_QUEUE_SIZE;
    }

    // Z80 /INT = High
    Z80_EXTINT_High();
    // Z80 int vector
    PORTA = vector;
    DDRA  = 0xff;

    // Clear /WAIT
    CLR_BIT(PORTD, PORTD5);
    asm("NOP");
    PORTA = 0xff;
    DDRA  = 0x00;
    SET_BIT(PORTD, PORTD5);

    // キューに残りがあれば再度 /INT をアサート
    if (int_queue_head != int_queue_tail) {
        z80_int_vector = int_queue[int_queue_head];
        CLR_BIT(PORTD, PORTD4);     // /INT = Low
    }
}
```

**既存の全 `Z80_EXTINT_low()` 呼び出しを `Z80_EXTINT_enqueue()` に置換**:

- `em_diskio.c`: `em_disk_read()`, `em_disk_write()` 内の 4 箇所
- `em_consoleio.c`: `Enqueue_RX1_Buf()`, `Transmit_TX1_Buf()` 内の 2 箇所

**`dbg_Continue()` については別途検討が必要**: デバッガはキューではなく即時送信が望ましいため、`Z80_EXTINT_low()` を残してもよい。ただし、キューとの整合性を確保するため、キューが空であることを前提条件とするか、キューをフラッシュしてから送信する方式とする。

**検証方法**: 
- コンパイル確認
- Zork I の長時間稼働テスト
- ディスク I/O 中にキー入力を繰り返してレース窓を意図的に狭めるストレステスト

---

#### Step 13: BUG #2 — `cli()`/`ExtMem_attach()` の順序修正

**対象ファイル**: `avr/src/emuldev/em_diskio.c`

**修正方針**: バス取得（`ExtMem_attach()`）を割り込み有効状態で行い、`cli()` は memcpy のみに限定する。

**修正パターン**: `em_disk_read()` と `em_disk_write()` の全 `cli()`/`ExtMem_attach()` ブロック（5 箇所）に適用。

**現行コード** (例: em_disk_read L564-568):

```c
cli();
ExtMem_attach();
memcpy(dst, tmpbuf, sizeof(tmpbuf));
ExtMem_detach();
sei();
```

**修正後コード**:

```c
ExtMem_attach();                // 割り込み有効のままバスを取得
uint8_t sreg = SREG;
cli();
memcpy(dst, tmpbuf, sizeof(tmpbuf));
SREG = sreg;
ExtMem_detach();
```

**適用箇所**（全 5 箇所）:

| ファイル | 行 | 関数 | 処理内容 |
|----------|-----|------|----------|
| em_diskio.c | L339-343 | em_disk_write | 先頭未アライン書き込み |
| em_diskio.c | L376-380 | em_disk_write | アライン済みブロック書き込み |
| em_diskio.c | L425-429 | em_disk_write | 末尾未アライン書き込み |
| em_diskio.c | L564-568 | em_disk_read | アライン済みブロック読み出し |
| em_diskio.c | L575-579 | em_disk_read | 端数読み出し |

**注意点**:

memcpy 中に割り込みを無効にする理由は、Z80 が同時にメモリにアクセスすることを防ぐためである。しかし、`ExtMem_attach()` でバスを取得した時点で Z80 はバスから外れているため、**cli() は実際には memcpy の保護ではなく、Timer2 再入によるバス操作の二重実行を防ぐため**と考えられる。Step 10（BUG #8 再入ガード）が適用済みであれば、Timer2 再入は発生しないため、`cli()` そのものが不要になる可能性がある。

ただし、安全のため SREG 保存/復元は残す。

**検証方法**: コンパイル確認。ディスク I/O の正常動作確認。特に Z80 が I/O WAIT 中にディスク I/O が発生するケース（CONIN 待機中のバックグラウンドディスク操作）を確認。

---

### Phase 3: 設計レベルの改善（Step 14）

Phase 2 完了後に着手する。

---

#### Step 14: BUG #7 — SELDSK の安全な状態リセット

**対象ファイル**: `avr/src/emuldev/em_diskio.c`

**修正方針**: `OUT_0A_DSK_SelectDisk()` で進行中の I/O が完了するまで待つ。ただし、この関数は INT1 ISR 内から呼ばれるため、単純な busy-wait では Timer2（`ISR_NOBLOCK`）がブロックされない限り問題ないが、設計の見直しが必要。

**案 A: 単純な待機（INT1 は通常 ISR だが Timer2 は ISR_NOBLOCK のため実行可能）**

```c
void OUT_0A_DSK_SelectDisk(uint8_t data)
{
    if (data < MAX_FILES) {
        cfd = &fd[data];
        // 進行中の操作の完了を待つ
        // INT1 は通常 ISR だが、Timer2 は ISR_NOBLOCK でありこの間も実行される
        while (cfd->read.state == DOING || cfd->write.state == DOING) {
            // Timer2 ISR (ISR_NOBLOCK) が操作を完了するのを待つ
        }
        ...（既存の re-open ロジック）...
        cfd->read.state   = IDLE;
        cfd->read.result  = FR_OK;
        cfd->write.state  = IDLE;
        cfd->write.result = FR_OK;
    }
}
```

**分析**: INT1 ISR は通常 ISR（割り込み無効）であるため、この while ループ中は Timer2 を含むすべての割り込みがブロックされる。Timer2 の `em_disk_read()`/`em_disk_write()` が実行できず、`DOING` 状態が解除されないため**デッドロックする**。

**案 B: INT1 ISR 内で一時的に割り込みを有効化して待機**

```c
void OUT_0A_DSK_SelectDisk(uint8_t data)
{
    if (data < MAX_FILES) {
        cfd = &fd[data];
        while (cfd->read.state == DOING || cfd->write.state == DOING) {
            sei();      // Timer2 に処理を完了させるため一時的に割り込み有効化
            asm("NOP");
            cli();
        }
        ...
    }
}
```

**分析**: 動作する可能性はあるが、INT1 ISR 内で `sei()` を呼ぶことにより、Z80 側の次の OUT 命令が処理される可能性がある（INT1 再入）。Z80 は BIOS の SELDSK 中であるため通常は次の OUT を発行しないが、保証はない。

**案 C: BIOS 側で SELDSK 前に WRITE 完了を確認する**

```asm
; bios.asm - SELDSK 修正
SELDSK:
        PUSH AF
        CALL WRITE_FLUSH           ; ← 既存コード：保留中の WRITE をフラッシュ
        ; WRITE_FLUSH は DISK_WRITE_SUB → WAIT_WRITE_COMPLETE で完了を待つ
        ; したがって、SELDSK 到達時には AVR 側の WRITE は完了済み
```

**分析**: 現行の `SELDSK` は既に `WRITE_FLUSH` を呼んでおり、完了を待ってから `OUT (PORT_SELDSK)` を発行している。つまり、**正常フローでは WRITE が DOING の状態で SELDSK に到達することはない**。

問題が起きるのは BUG #1（割り込み消失）により `WAIT_WRITE_COMPLETE` の `IS_WRITE_DONE` がセットされない場合である。この場合は Z80 がすでにスタックしているため、SELDSK に到達しない。

したがって、BUG #7 が顕在化する条件は限定的であり、**BUG #1 と BUG #8 の修正が完了すれば発生頻度は大幅に低下する**。

**推奨**: Phase 2 完了後に再評価する。BUG #1 修正後も再現が確認される場合に、案 C（BIOS 側での完了確認強化）を検討する。当面は `OUT_0A_DSK_SelectDisk()` に `DOING` 状態チェックのアサーション（デバッグログ出力）を追加し、発生有無を観測する。

**暫定修正**（観測用）:

```c
void OUT_0A_DSK_SelectDisk(uint8_t data)
{
    if (data < MAX_FILES) {
        cfd = &fd[data];
#if DEBUG_PRINT_STATE
        if (cfd->read.state == DOING || cfd->write.state == DOING) {
            x_printf("!!! SELDSK while I/O active: rd=%d wr=%d\n",
                     cfd->read.state, cfd->write.state);
        }
#endif
        ...（既存コード）...
    }
}
```

---

## 6. 修正順序のまとめ

```text
Phase 1（即着手可能、各ステップ独立）
├── Step 1:  BUG #4  — xconsoleio.c の SREG 保存/復元
├── Step 2:  BUG #11 — em_diskio.c の Status 関数 SREG 保存/復元
├── Step 3:  BUG #10 — em_diskio.c の volatile 追加
├── Step 4:  BUG #13 — xconsoleio.h の ConsoleBuffer volatile 追加
├── Step 5:  BUG #5  — em_consoleio.c のガード追加
├── Step 6:  BUG #12 — em_consoleio.c の Enqueue 戻り値チェック
├── Step 7:  BUG #14 — em_consoleio.c の非ブロッキング TX
├── Step 8:  BUG #15 — em_consoleio.c の通知閾値修正
├── Step 9:  BUG #6  — bios.asm LOAD_CCP_BDOS リトライ追加
├── Step 10: BUG #8  — isr.c Timer2 再入ガード追加
└── Step 11: BUG #9  — bios.asm DEBUGGER EI 復活

    ※ Step 5, 7, 8 は Transmit_TX1_Buf() を共通で修正

Phase 2（Phase 1 完了後）
├── Step 12: BUG #1  — z80io.c/isr.c 割り込みキュー化
│     前提: Step 1 (BUG #4), Step 2 (BUG #11)
└── Step 13: BUG #2  — em_diskio.c cli/ExtMem_attach 順序修正
      前提: Step 10 (BUG #8)

Phase 3（Phase 2 完了後、再評価）
└── Step 14: BUG #7 — em_diskio.c SELDSK 状態リセット安全化
      前提: Step 12 (BUG #1), Step 10 (BUG #8)
```

---

## 7. 検証計画

### Phase 1 完了後の検証

- BIOS アセンブル・AVR ファームウェアコンパイルが通ること
- CP/M 起動・基本操作（DIR, TYPE, PIP）の正常動作確認
- 9600 bps / 19200 bps それぞれでのコンソール入出力テスト
- TYPE コマンドによるバースト出力のスループット確認（BUG #14, #15 修正効果）
- Zork I の 30 分以上の連続プレイテスト

### Phase 2 完了後の検証

- Zork I の 2 時間以上の連続プレイテスト（BUG #1 修正前と同条件）
- ディスク I/O 中にキー入力を繰り返すストレステスト
- TinyMonitor からのブレークポイント設定・復帰テスト（BUG #9 検証）

### Phase 3 完了後の検証

- SELDSK の DEBUG_PRINT_STATE ログで DOING 状態での SELDSK 到達が報告されないことを確認
- 長時間稼働テスト

---

## 8. デバッグ手順（障害再現時）

Zork I 等の長時間動作中にシステムが無応答になった場合、以下の手順で障害の種類を特定する。

### 即座に確認すべき事項

1. **`/HALT` ピン（PB7）の状態確認**:
   - LOW: Z80 が真の HALT 命令を実行している → BOOT_ERROR_HALT (0xF436) または DEBUGGER (0xF292)
   - HIGH: ビジーウェイトループまたは WAIT 状態での停止

2. **コンソール出力の確認**: "System HALT due to CCP+BDOS load error." が表示されていないか確認。この文字列が確認できれば、BOOT_ERROR_HALT (0xF436) 経由であることが確定する。

3. **AVR TinyMonitor からのメモリダンプ**:
   - Z80 アドレス 0x0000-0x0002: `C3 xx xx` (JP WBOOT) なら正常。`76` なら TPA 内の HALT を実行
   - Z80 アドレス 0xF292: DEBUGGER の HALT に到達したか確認
   - Z80 アドレス 0xF436: BOOT_ERROR_HALT 周辺のコードが正常か確認

4. **ISR フラグの状態確認**:
   - アドレス 0xF314 (`IS_READ_DONE`): 0 なら `WAIT_READ_COMPLETE` で停止（BUG #1 の可能性）
   - アドレス 0xF31F (`IS_WRITE_DONE`): 0 なら `WAIT_WRITE_COMPLETE` で停止（BUG #1 の可能性）
