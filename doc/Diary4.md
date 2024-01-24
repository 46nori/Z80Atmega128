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
