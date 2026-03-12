#

## [以前の日記](Diary3.md)

## 2024/1/15

些細なドキュメントのバグを修正。

## 2024/1/21

CP/MのDISKイメージを操作する[cpmtools](http://www.moria.de/~michael/cpmtools/)、DISKのフォーマットが`/etc/cpmtools/diskdefs`に定義されているものしか指定できず不便。ドキュメントを読んでも任意の`diskdefs`を指定する方法がなさそうなので、いっちょ修正してやろうかとソースを読み始めた。

cpmfs.c

```c
  if ((fp=fopen("diskdefs","r"))==(FILE*)0 && (fp=fopen(DISKDEFS,"r"))==(FILE*)0)
  {
    fprintf(stderr,"%s: Neither `diskdefs' nor `" DISKDEFS "' could be opened.\n",cmd);
    exit(1);
  }
```

なんだ、カレントディレクトリに自分好みの`diskdefs`を置いとけばいいのか...。  
ということで、[Issue#13](https://github.com/46nori/Z80Atmega128/issues/13)を修正した。

## 2024/1/23

**「入門/実習/応用 CP/M」 村瀬康治 著 アスキー出版局**
この3部作は定番の参考書だった。だが1981,1982年の出版なので、もはや入手は難しい。ところが[INTERNET ARCHIVE](https://archive.org/details/cp-m-ascii-series)に収録されているのをたまたま発見した。ありがたや。

## 2024/4/20

あぁ、ついにZ80がディスコンに...。

- [Z84C00 End of Life/last Time Buy Notification](https://www.mouser.com/PCN/Littelfuse_PCN_Z84C00.pdf)
- [The legendary Zilog Z80 CPU is being discontinued after nearly 50 years](https://www.techspot.com/news/102684-zilog-discontinuing-z80-microprocessor-after-almost-50-years.html)

## 2024/8/25

CP/Mを再配布していものかはっきりしなかったので、CP/M本体は[The Unofficial CP/M Web site](http://www.cpm.z80.de/)から、オンデマンドでコピーしてビルドするようにしている。

しかし、本プロジェクト開始のわずか約半年前(2022/7/9)にClarifyされ、実はライセンス問題は解決してたことに気づいた。

- [CP/M's open-source status clarified after 21 years](https://www.theregister.com/2022/07/15/cpm_open_source/)
- [License agreement for the CP/M material presented on this site](http://www.cpm.z80.de/license.html)

## 2026/3/12

CP/M上で動くZORKを、AIで自律攻略することを思いついた。
以下のような構成で、MCPサーバー経由でZ80ATmega128を制御できるようにし、GitHub CopilotのAIエージェントがゲームを攻略させる。

```text
GitHub Copilot → MCP Server → Z80ATmega128
```

ということで、まずはシリアル経由でMCPサーバーを作った。  
GitHub: [mcp-serial-bridge](https://github.com/46nori/mcp-serial-bridge)

VSCodeのチャットからプロンプトを読み込ませ、チャットでプロンプトを読み込ませて指示を行う。

```text
#file:prompt_zork1.md を読んで、ゲームの攻略を開始してください。
```

LLMの種類によってかなり動きが異なる。

コンテキストサイズ、能力面からClaude Opus 4.6を試してみたが、プラムアムライセンスを三倍消費するくせに、シリアルデータをうまく取り込むことからおぼつかなかった。Claude Sonnet 4.6の方がだいぶましだった。GPT-5 miniは、いちいち指示待ちで、自律動作には程遠い。GPT-5.4が一番バランスがとれている印象。

### 攻略の様子

[![Zork攻略](https://img.youtube.com/vi/9ECWXsUKDJA/hqdefault.jpg)](https://youtu.be/9ECWXsUKDJA)

LLMモデルがGPT-5.4のときの攻略の様子。  
最終的にCP/M側がハングアップしてしまった。(T_T)

### プロンプト: prompt_zork1.md

```text
# Zorkの自律攻略

あなたはテキストアドベンチャーゲーム「Zork I」を攻略する自律プレイヤーです。人間の力を借りずに、ゲームと対話しながら自力で攻略してください。

## ゲームの起動

MCPサーバー(mcp-serial-bridge)を使用し、シリアルポートに接続されているCP/Mマシンを遠隔操作します。

1. ポートの一覧を取得して、適切なポートに 19200bps で接続してください。
2. "DIR"コマンドでファイル一覧を取得します。十分なタイムアウトを設定し、プロンプト'>'を待つようにしてください。
3. ファイル一覧が取得できたら、"ZORK1"を実行し、ゲームを起動します。十分なタイムアウトを設定し、プロンプト'>'を待つようにしてください。
4. ゲームが起動したら、以下の指針に従い、ゲームの攻略を開始してください。

## 目的

世界を探索し、アイテムを収集し、パズルを解き、可能な限りスコアを最大化してください。
あなたには現在のゲーム状態の要約が与えられます。
その情報をもとに状況を分析し、次に実行するコマンドを決定してください。

## 重要な行動方針

1. 新しい場所やオブジェクトを発見する探索行動を優先する
2. 見えているオブジェクトには積極的にインタラクションする
3. 同じ行動を繰り返して進展がない場合は別の行動を選択する
4. 新しい出口がある場合は移動を試す
5. アイテムは可能なら取得する
6. 無意味なコマンドの繰り返しを避ける
7.人間のインタラクションを求めない。自律的に行動を繰り返す。

## コマンド制約

出力するコマンドは必ずZorkの有効なコマンド形式にしてください。
使用可能なコマンド例:
look
inventory
north
south
east
west
up
down
open <object>
take <object>
drop <object>
read <object>
attack <target>
enter <location>

## 思考プロセス

以下の手順で考えてください。
1. 現在の状況を短く分析する
2. 可能な行動候補を3つ考える
3. その中から最も有望な行動を選択する

## 行動選択のルール
- 同じコマンドが直近2回実行され進展がない場合は選ばない
- 新しい場所や情報が得られる可能性の高い行動を優先する
- 可能なら未探索の出口を試す
- 見えているオブジェクトには優先的に行動する

## 出力フォーマット（必ずこの形式で出力すること）

Thought:
現在の状況の簡潔な分析

Candidate Actions:
1. <候補コマンド>
2. <候補コマンド>
3. <候補コマンド>

Best Action:
<選択したコマンド>

Command:
<実行するコマンド>

## 追加ルール

- 出力するコマンドは必ず1つだけにする
- コマンドは短く簡潔にする
- 無効なコマンドを作らない
- 同じ行動の無限ループを避ける

```
