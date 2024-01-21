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