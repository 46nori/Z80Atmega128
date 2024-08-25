## [以前の日記](Diary3.md)

## 2024/1/15
些細なドキュメントのバグを修正。

## 2024/1/21
CP/MのDISKイメージを操作する[cpmtools](http://www.moria.de/~michael/cpmtools/)、DISKのフォーマットが`/etc/cpmtools/diskdefs`に定義されているものしか指定できず不便。ドキュメントを読んでも任意の`diskdefs`を指定する方法がなさそうなので、いっちょ修正してやろうかとソースを読み始めた。

cpmfs.c
```
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
- [License agreement for the CP/M material presented on this site](http://)www.cpm.z80.de/license.html


