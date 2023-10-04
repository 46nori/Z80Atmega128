# KiCadによるPCBの製作

## 全体の流れ
### 1. 回路図の作成
1. 回路図エディタで回路を作成。
2. `ツール > 回路図をアノテーション`で部品のリファレンス番号をリセットする。
3. `検査 > エレクトリカル ルール チェッカー(ERC)`でエラーがないことを確認する。
4. `ツール > フットプリントを割り当て`で、部品のフットプリントをすべて定義する。

### 2. PCBの作成
1. 回路図エディタの`ツール > 回路図から基板を更新`でPCBを生成。フットプリントが不足している場合は定義する。
2. PCBエディタの`検査 > デザインルール チェッカー(DRC)`でエラーがないことを確認する。
3. 部品の位置決め
   - フットプリントを配置する。
   - おおよその位置が決まったら基板外形を決め、以後は、この中に収まるように作業する。
4. 基板設定
5. ベタの設定
   - In2.CuをVCCに、F.CU, In1.Cu, B.CuをGNDベタに設定する。
6. 手配線
   - 電源、パスコンなど自動配線だと支障があるものは手配線する。ロックしておくと良い。
   - シルクの文字の調整
7. 自動配線
   - Freeroutingのプラグインを使用。
   - それなりに賢く配線してくれるが、気に食わないところは手配線で直す。
8.  DRCで確認。問題があれば4,5, 6に戻る。
    - 基本、これを何度も繰り返す。
    - 回路図の修正が必要になることもある。この場合は基板への反映を忘れないこと。
9. GNDビア打ち
    - 配線が完成し、もう大修正を行わないことが前提。
    - GNDの貫通ビアを打ってGNDのインピーダンスを下げる。

### 3. 基板メーカーへの発注
1. `ファイル > 製造用出力 > ガーバー` : ガーバーデータ出力
2. `ファイル > 製造用出力 > ドリル出力` : ドリルデータ出力
3. 1, 2を.zipで固めてPCBメーカーに発注


## はじめにやっておくこと
### 自動配線の設定
  - `プラグイン&コンテンツ マネージャー`でFreeroutingプラグインをインストールする。

### ユーザーグリッドの設定
  - 0.1mmのグリッドを定義しておく。基板外形やネジ位置などmm指定したい場合、この設定に切り替えると作業がしやすい。
  - フットプリントや配線作業ではmilに切り替えて作業する。

## 各種設定の詳細
### 基板の設定
`ファイル > 基板の設定`
- `物理スタックアップ`
  - `導体レイヤー`で4層基板に設定
  - 使用するレイヤー
    | レイヤー      | 用途         |
    | ------------ | ----------- |
    | F.Cu         | 表面 配線     |
    | In1.Cu       | ベタ配線(GND) |
    | In2.Cu       | ベタ配線(VCC) |
    | B.Cu         | 裏面 配線     |
    | F.Silkscreed | 表面シルク    |
    | B.Silkscreed | 裏面シルク    |
    | Edge.Cuts    | 基板の外形定義 |
    | User.Drawings| 基板の外形寸法 |

- `デザインルール＞制約`
  - 最小クリアランス: 0.1524 (6mil)
  - 最小配線幅: 0.2mm
  - 最小アニューラ幅: 0.2mm
  - 最小ビア直径: 0.4mm
  - 導体から穴のクリアランス: 0.25mm
  - 導体から基板端クリアランス: 1mm
  - 最小スルーホール: 0.3mm
  - 穴から穴へのクリアランス: 0.25mm
  - 最小サーマルリーフスポーク数: 1  
    デフォルトは2。ベタのGND接続がスポーク1本で繋がってしまう場合が結構あり、以下のエラーが発生する。
    ```
    エラー: ゾーンへのサーマルリーフ接続が不完全...塗りつぶし方法の制約 最小スポーク数: 2; 現状1)
    ```

### 基板外形の設定
- Edge.Cutsレイヤに基板の外形を描く。
- ネジ穴の配置
  - 銅箔付きのM3のPadを選択 `MountingHole_3mm_M3_ISO7380_Pad`
  - 周辺の領域をキープアウトに設定し、VCCレイヤーでベタ塗りを禁止する。

### ベタの設定
- 設定
  `配置 > 塗りつぶしゾーンを追加`で、ベタを設定する領域を指定する。複数のレイヤーがまとめて指定できるので、ベタGNDの設定はこれが便利。
- 除外
  - `ルールアウト(キープアウト)を追加`で、ベタを設定しない領域を指定できる。
- 一時的な削除
  - DRCをかけるとデフォルトでベタを生成してくれるが、配線や目視チェックで邪魔になるので、一時的にベタを着したい場合がある。
  - `編集 > 全てのゾーンの塗りつぶしを削除`で消える。

### パスコンの配線
- パスコンからICのVCCに直接接続しないと意味がない。しかしVCCのベタを設定すると勝手に配線されてしまう。これを避けるため、ICのVCC端子周辺にベータのキープアウトエリアを配置する。ICのVCC端子は表面または裏面層でパスコンに接続する。

### 配線のやり直し
- 自動配線をやり直したい場合がある。配線を全消しすることもできるが、手配線は残したい。`編集>広域削除`で削除したいものを選択する。残しておきたい要素はロックをかけ、それ以外のものを削除するようにすると手戻りを減らせる。

### GNDビア
- 10mm程度の間隔で打つ
- 基板端にぐるっと等間隔に打つ
- 島になっているところはアンテナになるので、端に打って解放端の島にしない。
- まんべんなく打つ

### ティアドロップ
- `ツール > ティアドロップの追加`
- KiCad V7から入った機能。ビアやランドの付け根に涙のようなランドが付加される。

### 製造データ
- ドリルの原点の設定
  - `配置 > ドリル/配置ファイルの原点`で、基板左下に原点をセットする。
- ガーバー出力
    - `ドリル/配置ファイルの原点を使用`にチェックする。
- ドリル出力
  - `ドリル単位`を`mm`にする。
