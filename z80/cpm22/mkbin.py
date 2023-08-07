def main():
    with open("data", "wb") as fout:
        bary = bytearray(128)
        tmp = bytearray(128)
        for blk in range(1, 4):
            for idx in range(0, 128): 
                tmp[idx] = blk
            bary += tmp
        fout.write(bary)

if __name__ == "__main__":
    main()
