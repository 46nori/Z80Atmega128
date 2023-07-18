import sys

def generate_binary_file(file_path, file_size, data):
    with open(file_path, 'wb') as file:
        chunk_size = 512  # Chunk size of a write

        while file.tell() < file_size:
            remaining_bytes = file_size - file.tell()
            chunk = bytes.fromhex(data[2:4])[:min(chunk_size, remaining_bytes)]
            file.write(chunk)

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print('Usage: python3 mkimage.py <filename> <size> <data>')
        sys.exit(1)

    file_path = sys.argv[1]
    file_size = int(sys.argv[2])
    data = sys.argv[3]

    generate_binary_file(file_path, file_size, data)
